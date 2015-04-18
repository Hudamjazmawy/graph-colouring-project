# Introduction to git #

A brief note of how to set up and use git.

# Configuration #
```
echo machine code.google.com >> ~/.netrc
echo login email >> ~/.netrc
echo password password >> ~/.netrc
chmod go=~/.netrc
```
# Common operations #

The svn of this project repository is _git_:
  * To setup remote:
```
git remote add google https://project.googlecode.com/git
```
  * To add a project into the svn git:
```
git add projectname
```
  * To commit the changes in local copy:
```
git commit -a -m "message"
```
  * To commit the changes to google code:
```
git push google
```