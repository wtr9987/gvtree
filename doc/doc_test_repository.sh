#!/bin/sh

mkdir test_repository
cd test_repository
git init 
git branch -m master main
echo "int main(int argc, char* argv[]) { return 0; }" > main.c
git add main.c
git commit

git branch branch1
git checkout branch1
echo "YYY" > README
git add README
git commit

git checkout main
echo "XXX" > README
git add README
git commit

git merge branch1
git add README
git commit

git tag -a -m "STR1234_HO" STR1234_HO

echo "TODO" > TODO
git add TODO
git commit

echo "TODO" >> TODO
git add TODO
git commit

git checkout STR1234_HO
git branch branch2
git checkout branch2
echo "ChangeLog" > ChangeLog
git add ChangeLog
git commit

git checkout main

