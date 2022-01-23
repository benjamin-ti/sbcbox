
# Stdlayout means trunk/branch/tags
# modify paths and adresses of this script
# Call from empty target directory, i.e. cd vetogit; bash ../svn2git.sh
git svn clone --stdlayout --authors-file=../authors.txt https://cvsvn/svn/Veto .

# this converts branches and tags *only* to remote
# To do it better use svn2git (git-svn is still needed):
# sudo apt-get install ruby
# sudo gem install svn2git
# svn2git https://cvsvn/svn/Veto

# git branch --track "P0.3" "remotes/svn/P0.3"
# git checkout -t "remotes/svn/P0.3"
