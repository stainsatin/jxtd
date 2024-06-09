# development workflow
1. (initial)fork framework/jxtd(called 'upstream repository') to own namespace(called 'personal repository')
2. (initial)clone personal repository into local(called 'local repository')
3. checkout into/create new branch
4. (daily)update latest commits from upstream(for both main and development branches)
5. coding & commit
6. push dev branch to personal repository
7. create mr and waiting for being merged

# update latest commits
1. checkout into main branch
2. fetch the latest upstream commits & merge into local main branch
3. checkout into dev branch
4. rebase on main branch 
5. push dev branch to personal repository via forcing push
