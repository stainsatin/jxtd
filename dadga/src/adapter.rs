use std::collections::HashMap;
use std::error::Error;
use std::io::Error as ioError;
use tokio::io::AsyncWriteExt;
use std::io::ErrorKind;

pub struct FsAdapter{
	root: String,
	persistent: String,
	runtime: String,
	//(taskid, task_root_path)
	tasks: HashMap<String, String>
}

impl FsAdapter{
	//对于客户端，
	//persistent持久化路径和root路径为%root%/persistent，
	//runtime运行时路径为%root%/runtime。

	//对于服务端，
	//persistent和runtime均为root。
	pub fn new(root: &str, client: bool) -> Result<Self, Box<dyn Error>> {
		let (persistent,runtime) = match client {
			true => (format!("{}/persistent", root),format!("{}/runtime", root)),
            false => (root.to_string(),root.to_string())
        };

		//根据root创建运行时数据区，若root路径存在则清空后再创建路径，若root不存在则创建root目录。
		if std::path::Path::new(root).exists() {
			std::fs::remove_dir_all(root)?;
		}
		std::fs::create_dir_all(root)?;
		//同时创建persistent和runtime对应路径，处理策略同root。
        std::fs::create_dir_all(&persistent)?;
        std::fs::create_dir_all(&runtime)?;

		Ok(Self {
            root: root.to_string(),
            persistent,
            runtime,
            tasks: HashMap::new(),
        })
	}

	//读取文件。
	//path是相对于root的路径。
	pub async fn read_file(&self, path: &str) -> Result<String, ioError> {
		let file_path = format!("{}/{}", self.root, path);
		tokio::fs::read_to_string(file_path).await
	}

	//读取persistent区域文件。
	//path：相对于persistent路径。
	pub async fn read_persistent(&self, path: &str) -> Result<String, ioError> {
        let file_path = format!("{}/{}", self.persistent, path);
        tokio::fs::read_to_string(file_path).await
    }

	//创建一个task运行时路径。
	//在runtime目录下创建taskid专属根路径task_root_path，
	//向tasks添加(taskid, task_root_path)。
	//若taskid已存在，返回Ok但不会做任何事（静默成功）。
	pub fn create_task(&mut self, taskid: &str) -> Result<(), Box<dyn Error>> {
        if self.tasks.contains_key(taskid) {
            return Ok(());
        }
        let task_root_path = format!("{}/{}", self.runtime, taskid);
        std::fs::create_dir_all(&task_root_path)?;
        self.tasks.insert(taskid.to_string(), task_root_path);
        Ok(())
    }

	//清除taskid对应运行时路径。
	//若taskid不存在则静默成功。
	pub fn task_done(&mut self, taskid: &str) -> Result<(), Box<dyn Error>> {
		std::fs::remove_dir_all(format!("{}/{}", self.runtime, taskid))?;
		self.tasks.remove(taskid);
        Ok(())
    }

	//读取taskid对应task的运行时数据。
	pub async fn read_task_file(&self, taskid: &str, path: &str) -> Result<String, ioError> {
        let task_root_path = match self.tasks.get(taskid) {
                Some(x) => x.to_string(),
                None =>  return Err(ioError::new(ErrorKind::NotFound, ""))
            };
        let file_path = format!("{}/{}", task_root_path, path);
        tokio::fs::read_to_string(file_path).await
    }

	//写入文件。
	//path: 相对于root的路径。
	//write_file：若path存在，则截断并覆盖。
	//write_file_exclusive：若path存在，则错误。
	//write_file_append：若path存在，则追加。
	pub async fn write_file(&self, path: &str, content: &[u8]) -> Result<(), Box<dyn Error>> {
        let file_path = format!("{}/{}", self.root, path);
        tokio::fs::OpenOptions::new()
            .write(true)
            .create(true)
            .truncate(true)
            .open(file_path)
            .await?
            .write_all(content).await?;
        Ok(())
    }
	pub async fn write_file_exclusive(&self, path: &str, content: &[u8]) -> Result<(), Box<dyn Error>> {
        let file_path = format!("{}/{}", self.root, path);
        tokio::fs::OpenOptions::new()
            .write(true)
            .create_new(true)
            .open(file_path)
            .await?
            .write_all(content).await?;
        Ok(())
    }
	pub async fn write_file_append(&self, path: &str, content: &[u8]) -> Result<(), Box<dyn Error>> {
        let file_path = format!("{}/{}", self.root, path);
        tokio::fs::OpenOptions::new()
            .create(true)
            .append(true)
            .open(file_path)
            .await?
            .write_all(content)
            .await?;
        Ok(())
    }

	//和read对应的各种写入。
	pub async fn write_persistent(&self, path: &str, content: &[u8]) -> Result<(), Box<dyn Error>> {
        let file_path = format!("{}/{}", self.persistent, path);
        tokio::fs::OpenOptions::new()
            .write(true)
            .create(true)
            .truncate(true)
            .open(file_path)
            .await?
            .write_all(content).await?;
        Ok(())
    }
	pub async fn write_persistent_exclusive(&self, path: &str, content: &[u8]) -> Result<(), Box<dyn Error>> {
        let file_path = format!("{}/{}", self.persistent, path);
        tokio::fs::OpenOptions::new()
            .write(true)
            .create_new(true)
            .open(file_path)
            .await?
            .write_all(content).await?;
        Ok(())
    }
	pub async fn write_persistent_append(&self, path: &str, content: &[u8]) -> Result<(), Box<dyn Error>> {
        let file_path = format!("{}/{}", self.persistent, path);
        tokio::fs::OpenOptions::new()
            .create(true)
            .append(true)
            .open(file_path)
            .await?
            .write_all(content)
            .await?;
        Ok(())
    }

	pub async fn write_task_file(&self, taskid: &str, path: &str, content: &[u8]) -> Result<(), ioError> {
        let task_root_path = match self.tasks.get(taskid) {
            Some(x) => x.to_string(),
            None =>  return Err(ioError::new(ErrorKind::NotFound, ""))
        };
        let file_path = format!("{}/{}", task_root_path, path);
        tokio::fs::OpenOptions::new()
            .write(true)
            .create(true)
            .truncate(true)
            .open(file_path)
            .await?
            .write_all(content).await?;
        Ok(())
    }
	pub async fn write_task_file_exclusive(&self, taskid: &str, path: &str, content: &[u8]) -> Result<(), ioError> {
        let task_root_path = match self.tasks.get(taskid) {
            Some(x) => x.to_string(),
            None =>  return Err(ioError::new(ErrorKind::NotFound, ""))
        };
        let file_path = format!("{}/{}", task_root_path, path);
        tokio::fs::OpenOptions::new()
            .write(true)
            .create_new(true)
            .open(file_path)
            .await?
            .write_all(content).await?;
        Ok(())
    }
	pub async fn write_task_file_append(&self, taskid: &str, path: &str, content: &[u8]) -> Result<(), ioError> {
        let task_root_path = match self.tasks.get(taskid) {
            Some(x) => x.to_string(),
            None =>  return Err(ioError::new(ErrorKind::NotFound, ""))
        };
        let file_path = format!("{}/{}", task_root_path, path);
        tokio::fs::OpenOptions::new()
            .create(true)
            .append(true)
            .open(file_path)
            .await?
            .write_all(content)
            .await?;
        Ok(())
    }

	//文件删除，静默成功
	pub async fn remove_file(&self, path: &str) -> Result<(), ioError> {
        let file_path = format!("{}/{}", self.root, path);
        if tokio::fs::metadata(&file_path).await.is_ok() {
            return tokio::fs::remove_file(file_path).await;
        }
        Ok(())
    }
	pub async fn remove_persistent(&self, path: &str) -> Result<(), ioError> {
        let file_path = format!("{}/{}", self.persistent, path);
        if tokio::fs::metadata(&file_path).await.is_ok() {
            return tokio::fs::remove_file(file_path).await;
        }
        Ok(())
    }
	pub async fn remove_task_file(&self, taskid: &str, path: &str) -> Result<(), ioError> {
        let task_root_path = match self.tasks.get(taskid) {
            Some(x) => x.to_string(),
            None =>  return Err(ioError::new(ErrorKind::NotFound, ""))
        };
        let file_path = format!("{}/{}", task_root_path, path);
        if tokio::fs::metadata(&file_path).await.is_ok() {
            return tokio::fs::remove_file(file_path).await;
        }
        Ok(())
    }

	//复制persistent中%persistent%路径的文件至taskid对应task的%task_path%路径，覆盖策略。
	pub async fn persistent_to_runtime(&self, persistent: &str, taskid: &str, task_path: &str) -> Result<u64, ioError> {
        let persistent_file_path = format!("{}/{}", self.persistent, persistent);
        let task_root_path = match self.tasks.get(taskid) {
            Some(x) => x.to_string(),
            None =>  return Err(ioError::new(ErrorKind::NotFound, ""))
        };
        let task_file_path = format!("{}/{}", task_root_path, task_path);
        tokio::fs::copy(persistent_file_path, task_file_path).await
    }
	//反方向复制，覆盖策略。
	pub async fn runtime_to_persistent(&self, persistent: &str, taskid: &str, task_path: &str) -> Result<u64, ioError> {
        let persistent_file_path = format!("{}/{}", self.persistent, persistent);
        let task_root_path = match self.tasks.get(taskid) {
            Some(x) => x.to_string(),
            None =>  return Err(ioError::new(ErrorKind::NotFound, ""))
        };
        let task_file_path = format!("{}/{}", task_root_path, task_path);
        tokio::fs::copy(task_file_path, persistent_file_path).await
    }
}

#[cfg(test)]
mod test {
	use std::fs;
    use super::*;
	
	#[tokio::test]
	async fn test_client() {
        //client视角下只有最最简单的测试，其余的在sever部分，就不重复了
		let root = "./test";//是这样吗？
		let mut fsadapter1 = FsAdapter::new(root, true).unwrap();
		assert_eq!(fsadapter1.root, "./test");
		assert_eq!(fsadapter1.persistent, "./test/persistent");
		assert_eq!(fsadapter1.runtime, "./test/runtime");
        //验证persistent和runtime目录是否正确创建
        assert!(fs::metadata(fsadapter1.persistent.clone()).is_ok());
        assert!(fs::metadata(fsadapter1.runtime.clone()).is_ok());
        
        let content = b"Hello, FsAdapter!";
        let content_copy = b"This is text for copy!";
        let file_path_copy = "Copy.md";//放在persistent

        //root
        let file_path_root = "test_file_root.txt";                                               
        fsadapter1.write_file(file_path_root, content).await.unwrap();
        let read_content = fsadapter1.read_file(file_path_root).await.unwrap();
        assert_eq!(read_content, String::from_utf8_lossy(content));
        
        //persistent
        let file_path_persistent = "persistent_test_file.txt";
        fsadapter1.write_persistent(file_path_persistent, content).await.unwrap();
        let persistent_read_content = fsadapter1.read_persistent(file_path_persistent).await.unwrap();
        assert_eq!(persistent_read_content, String::from_utf8_lossy(content));
        
        //tasks
        let tid = "task1";
        fsadapter1.create_task(tid).unwrap();
        assert!(fsadapter1.tasks.contains_key(tid));
        let file_path_task = "task_test_file.md";
        fsadapter1.write_task_file(tid, file_path_task, content).await.unwrap();
        let task_read_content = fsadapter1.read_task_file(tid, file_path_task).await.unwrap();
        assert_eq!(task_read_content, String::from_utf8_lossy(content));
        
        //复制
        fsadapter1.write_persistent(file_path_copy, content_copy).await.unwrap();
        fsadapter1.persistent_to_runtime(file_path_copy, tid, file_path_task).await.unwrap();
        let read_runtime_copy = fsadapter1.read_task_file(tid, file_path_task).await.unwrap();
        assert_eq!(read_runtime_copy, String::from_utf8_lossy(content_copy));

        fsadapter1.runtime_to_persistent(file_path_persistent, tid, file_path_task).await.unwrap();
        let read_persistent_copy = fsadapter1.read_persistent(file_path_persistent).await.unwrap();
        assert_eq!(read_persistent_copy, String::from_utf8_lossy(content_copy));

        //let copy_in_persistent = format!("{}/{}", fsadapter.persistent, );

        //任务完成
        fsadapter1.task_done(tid).unwrap();
        assert!(!fsadapter1.tasks.contains_key(tid));
        assert!(!fs::metadata(format!("{}/{}", fsadapter1.runtime, tid)).is_ok());
        std::fs::remove_dir_all(root).unwrap();
	}

    #[tokio::test]
    async fn test_sever() {
        let root = "./test1";
		let mut fsadapter = FsAdapter::new(root, false).unwrap();
		assert_eq!(fsadapter.root, "./test1");
		assert_eq!(fsadapter.persistent, "./test1");
        assert_eq!(fsadapter.persistent, fsadapter.runtime);
        //验证persistent和runtime目录是否正确创建
        assert!(fs::metadata(fsadapter.persistent.clone()).is_ok());
        assert!(fs::metadata(fsadapter.runtime.clone()).is_ok());

        let content = b"Hello, FsAdapter!";
        let content_append = b"This is appended text!";
        let content_after_ap = b"Hello, FsAdapter!This is appended text!";
        let content_remove = b"And test removal!";
        let file_path_remove = "test_file_remove";

        //root
        let file_path_root = "test_file_root.txt";
        //write_file                                           
        fsadapter.write_file(file_path_root, content).await.unwrap();
        let read_content = fsadapter.read_file(file_path_root).await.unwrap();
        assert_eq!(read_content, String::from_utf8_lossy(content));
        //write_file_append
        fsadapter.write_file_append(file_path_root, content_append).await.unwrap();
        let read_content_append = fsadapter.read_file(file_path_root).await.unwrap();
        assert_eq!(read_content_append, String::from_utf8_lossy(content_after_ap));
        //write_file_exclusive
        let result_root = fsadapter.write_file_exclusive(file_path_root, content_remove).await;
        assert!(result_root.is_err());//第二次写入应该失败，下同
        let result_root2 = fsadapter.write_file_exclusive(file_path_remove, content_remove).await;
        assert!(result_root2.is_ok());//第一次写入应该成功，下同

        //persistent
        let file_path_persistent = "persistent_test_file.txt";
        //write_file        
        fsadapter.write_persistent(file_path_persistent, content).await.unwrap();
        let persistent_read_content = fsadapter.read_persistent(file_path_persistent).await.unwrap();
        assert_eq!(persistent_read_content, String::from_utf8_lossy(content));                                                  
        //write_file_append
        fsadapter.write_persistent_append(file_path_persistent, content_append).await.unwrap();
        let persistent_content_append = fsadapter.read_persistent(file_path_persistent).await.unwrap();
        assert_eq!(persistent_content_append, String::from_utf8_lossy(content_after_ap));
        //write_file_exclusive
            //测试remove：利用在sever里root=persistent
            //由于之前root写入了file_path_remove，所以这里是写不进去的
        let result_persistent = fsadapter.write_persistent_exclusive(file_path_remove, content_remove).await;
        assert!(result_persistent.is_err());
            //同理，用persistent的函数删除root写入的file_path_remove后，persistent的write才能写入file_path_remove
            //write_persistent_exclusive的特性已经证明，所以remove_persistent也通过！！！下同
        fsadapter.remove_persistent(file_path_remove).await.unwrap();
        let result_persistent2 = fsadapter.write_persistent_exclusive(file_path_remove, content_remove).await;
        assert!(result_persistent2.is_ok());
            //同理，用root的函数删除persistent写入的file_path_persistent后，root的write才能写入file_path_persistent
        let result_persistent3 = fsadapter.write_file_exclusive(file_path_persistent, content_remove).await;
        assert!(result_persistent3.is_err());
            //remove_file的可用性在此证明
        fsadapter.remove_file(file_path_persistent).await.unwrap();
        let result_persistent4 = fsadapter.write_file_exclusive(file_path_persistent, content_remove).await;
        assert!(result_persistent4.is_ok());

        //tasks ,which is under runtime(root)
        let tid = "task1";
        let file_path_task = "task_test_file.md";        
        fsadapter.create_task(tid).unwrap();
        assert!(fsadapter.tasks.contains_key(tid));
        //write_file        
        fsadapter.write_task_file(tid, file_path_task, content).await.unwrap();
        let task_read_content = fsadapter.read_task_file(tid, file_path_task).await.unwrap();
        assert_eq!(task_read_content, String::from_utf8_lossy(content));
        //write_file_append
        fsadapter.write_task_file_append(tid, file_path_task, content_append).await.unwrap();
        let task_content_append = fsadapter.read_task_file(tid, file_path_task).await.unwrap();
        assert_eq!(task_content_append, String::from_utf8_lossy(content_after_ap));
        //write_file_exclusive
        let result_task = fsadapter.write_task_file_exclusive(tid, file_path_task, content_remove).await;
        assert!(result_task.is_err());
        let result_task2 = fsadapter.write_task_file_exclusive(tid, file_path_remove, content_remove).await;
        assert!(result_task2.is_ok());//同上（root）
        //remove_task_file
        fsadapter.remove_task_file(tid, file_path_task).await.unwrap();
        let result_task3 = fsadapter.write_task_file_exclusive(tid, file_path_task, content_remove).await;
        assert!(result_task3.is_ok());

        //复制，由于重复性（在client内），不在此列举
        
        //任务完成
        fsadapter.task_done(tid).unwrap();
        assert!(!fsadapter.tasks.contains_key(tid));
        assert!(!fs::metadata(format!("{}/{}", fsadapter.runtime, tid)).is_ok());
        std::fs::remove_dir_all(root).unwrap();
    }
}