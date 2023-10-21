import argparse, logging, traceback
import os, sys, shutil
import subprocess as sp

logging.basicConfig(level=logging.INFO)

app = ""
gitdir = ".s-git/"

class TestFailed(Exception):
  pass

# helper function
def CheckOutputContain(res, msg):
  if res.stdout.find(msg) == -1:
    logging.error(f"want {msg}")
    logging.error(f"output {res.stdout}")
    raise TestFailed

def CheckOutputCount(res, msg, time):
  if res.stdout.count(msg) != time:
    raise TestFailed

def CheckFileExact(file, content):
  try:
    with open(file) as f:
      data = f.read()
      if data != content:
        logging.error(f"want {content}")
        logging.error(f"got {data}")
        raise TestFailed
  except FileNotFoundError:
    raise TestFailed

def CheckFileExist(file, true = True):
  if os.path.exists(file) != true:
    raise TestFailed

def CheckFileSystemFirst():
  CheckFileExact("dir/b.txt", "")
  CheckFileExact("a.txt", "")
  CheckFileExist("dir2/c.txt", False)

def CheckFileSystemSecond(clean = True):
  CheckFileExist("dir/b.txt", False)
  CheckFileExact("a.txt", "test")
  CheckFileExact("dir2/c.txt", "")
  if clean:
    CheckFileExist("dir", False)

def CheckIgnore():
  CheckFileExist("dir2")
  CheckFileExist("dir2/ignore")
  CheckFileExist("dir2/ignore/.s-gitignore")

def ExtractCommitSha1(res):
  square = res.stdout.rfind(']')
  space = res.stdout.rfind(' ', 0, square)
  if space == -1 or square == -1:
    raise TestFailed
  return res.stdout[space + 1: square]

def ExtractLogSha1(res):
  sha1 = list()
  idx = 0
  while True:
    idx = res.stdout.find("[commit ", idx)
    if idx == -1:
      return sha1
    idx += len("[commit ")
    sha1.append(res.stdout[idx: idx + 40])
    idx += 40

def Touch(file):
  open(file, 'a').close()

def Echo(msg, file):
  with open(file, 'w') as f:
    f.write(msg)


# test function
def TestInit():
  res = sp.run([app, "init"], check=True, stdout=sp.PIPE, text=True)
  CheckOutputContain(res, "Initialized")
  CheckFileExact(gitdir + "HEAD", "ref: refs/heads/main")
  logging.info("'TestInit' passes")

def TestStatusNoCommit():
  res = sp.run([app, "status"], check=True, stdout=sp.PIPE, text=True)
  CheckOutputContain(res, "No commits yet")
  logging.info("'TestStatusNoCommit' passes")

def TestStatusUntracked():
  Touch('a.txt')
  os.mkdir('dir')
  Touch('dir/b.txt')
  res = sp.run([app, "status"], check=True, stdout=sp.PIPE, text=True)
  CheckOutputContain(res, "a.txt")
  CheckOutputContain(res, "dir/b.txt")
  CheckOutputCount(res, "dir", 1)
  logging.info("'TestStatusUntracked' passes")

def TestCommitFirst():
  res = sp.run([app, "commit", "-m", "first"], check=True, stdout=sp.PIPE, text=True)
  CheckOutputContain(res, "first commit")
  CheckOutputContain(res, "root-commit")
  CheckOutputContain(res, "2 file inserted")
  logging.info("'TestCommitFirst' passes")
  return ExtractCommitSha1(res)

def TestBranchAdd(name):
  res = sp.run([app, "branch", name], check=True, stdout=sp.PIPE, text=True)
  CheckOutputContain(res, "Added")
  logging.info("'TestBranchAdd' passes")

def TestBranchList(nameList):
  res = sp.run([app, "branch", "-l"], check=True, stdout=sp.PIPE, text=True)
  for name in nameList:
    CheckOutputContain(res, name)
  logging.info("'TestBranchList' passes")

def TestTagAdd(name):
  res = sp.run([app, "tag", name], check=True, stdout=sp.PIPE, text=True)
  CheckOutputContain(res, "Added")
  logging.info("'TestTagAdd' passes")

def TestTagList(nameList):
  res = sp.run([app, "tag", "-l"], check=True, stdout=sp.PIPE, text=True)
  for name in nameList:
    CheckOutputContain(res, name)
  logging.info("'TestTagList' passes")

def TestStatusChangeNewDelete():
  Echo("test", "a.txt")
  os.remove("dir/b.txt")
  os.mkdir("dir2")
  os.mkdir("dir2/ignore")
  Touch("dir2/ignore/.s-gitignore")
  Touch("dir2/c.txt")
  res = sp.run([app, "status"], check=True, stdout=sp.PIPE, text=True)
  CheckOutputContain(res, "On branch main")
  idxChange = res.stdout.find("changed")
  idxNew = res.stdout.find("new")
  idxDelete = res.stdout.find("delete")
  if idxChange == -1 or idxNew == -1 or idxDelete == -1:
    raise TestFailed
  if res.stdout.find("a.txt", idxChange, idxNew) == -1 or \
    res.stdout.find("dir2/c.txt", idxNew, idxDelete) == -1 or \
    res.stdout.find("dir/b.txt", idxDelete) == -1:
    raise TestFailed
  CheckOutputCount(res, ".s-gitignore", 0)
  logging.info("'TestStatusChangeNewDelete' passes")

def TestCommitSecond():
  res = sp.run([app, "commit", "-m", "second"], check=True, stdout=sp.PIPE, text=True)
  CheckOutputContain(res, "1 files changed")
  CheckOutputContain(res, "1 files inserted")
  CheckOutputContain(res, "1 files deleted")
  logging.info("'TestCommitSecond' passes")
  return ExtractCommitSha1(res)

def TestLog(msgs):
  res = sp.run([app, "log"], check=True, stdout=sp.PIPE, text=True)
  idx = 0
  for msg in msgs:
    idx = res.stdout.find(msg, idx)
    if idx == -1:
      raise TestFailed
    idx += len(msg)
  logging.info("'TestLog' passes")
  return ExtractLogSha1(res)

def TestShortSha1(shorts, longs):
  for i in range(0, len(longs)):
    if not longs[i].startswith(shorts[i]):
      raise TestFailed
  logging.info("'TestShortSha1' passes")

def TestCheckoutBranch(branch, first):
  res = sp.run([app, "checkout", "-b", branch], check=True, stdout=sp.PIPE, text=True)
  CheckOutputContain(res, f"Switched to branch '{branch}'")
  if first:
    CheckFileSystemFirst()
  else:
    CheckFileSystemSecond()
  CheckIgnore()
  logging.info("'TestCheckoutBranch' passes")

def TestBranchDeleteFail(branch):
  res = sp.run([app, "branch", "-d", branch], check=True, stdout=sp.PIPE, text=True)
  CheckOutputContain(res, "unable")
  logging.info("'TestBranchDeleteFail' passes")

def TestCheckoutTag(tag):
  res = sp.run([app, "checkout", "-t", tag], check=True, stdout=sp.PIPE, text=True)
  CheckOutputContain(res, "HEAD detached")
  CheckFileSystemFirst()
  CheckIgnore()
  logging.info("'TestCheckoutTag' passes")

def TestStatusDetached(sha1):
  res = sp.run([app, "status"], check=True, stdout=sp.PIPE, text=True)
  CheckOutputContain(res, f"detached at {sha1}")
  logging.info("'TestStatusDetached' passes")

def TestTagDelete(tag):
  res = sp.run([app, "tag", "-d", tag], check=True, stdout=sp.PIPE, text=True)
  CheckOutputContain(res, "Deleted")
  logging.info("'TestTagDelete' passes")

def TestBranchDelete(branch):
  res = sp.run([app, "branch", "-d", branch], check=True, stdout=sp.PIPE, text=True)
  CheckOutputContain(res, "Deleted")
  logging.info("'TestBranchDelete' passes")

def TestCheckoutSha(sha1):
  res = sp.run([app, "checkout", "-s", sha1], check=True, stdout=sp.PIPE, text=True)
  CheckOutputContain(res, f"HEAD detached at {sha1}")
  logging.info("'TestCheckoutSha' passes")
  CheckFileSystemFirst()

def TestCheckoutRevision(revision):
  res = sp.run([app, "checkout", "-b", revision], check=True, stdout=sp.PIPE, text=True)
  CheckOutputContain(res, "Switched")
  CheckFileSystemFirst()
  logging.info("'TestCheckoutRevision' passes")

# end of test function

def LoadTmpDir():
  tmp = "s-git_tmp"
  count = 0
  while count < 1000:
    path = tmp + str(count)
    if not os.path.exists(path):
      os.mkdir(path)
      os.chdir(path)
      logging.info(f"enter tmp directory {path}")
      return
    count += 1
  raise "too many tmp directory, please clear them before testing"

def DeleteTmpDir():
  dir = os.path.abspath(os.getcwd())
  logging.info(f"exit tmp directory {os.path.basename(dir)}")
  os.chdir("..")
  logging.info(f"remove tmp directory {dir}")
  shutil.rmtree(dir)

def main():
  try:
    TestInit()
    TestStatusNoCommit()
    TestLog(["second", "first"])
    TestStatusUntracked()
    shaFirstShort = TestCommitFirst()
    TestBranchAdd("master")
    TestBranchList(["main", "master"])
    TestTagAdd("first")
    TestTagList(["first"])
    TestStatusChangeNewDelete()
    shaSecondShort = TestCommitSecond()
    shaList = TestLog(["second", "first"])
    TestShortSha1([shaSecondShort, shaFirstShort], shaList)
    TestCheckoutBranch("master", True)
    TestBranchDeleteFail("master")
    TestCheckoutTag("first")
    TestStatusDetached(shaList[1])
    TestTagDelete("first")
    TestTagList([])
    TestBranchDelete("master")
    TestBranchList(["main"])
    TestCheckoutBranch("main", False)
    TestLog(["second", "first"])
    TestCheckoutSha(shaList[1])
    TestCheckoutRevision("main~1")
    return True
  except TestFailed as exc:
    exc_traceback = sys.exc_info()
    traceback.print_exc()
    stack = traceback.extract_tb(exc_traceback[-1])
    idx = -1
    while True:
      frame = stack[idx]
      funcName = frame.name
      if funcName.startswith("Test"):
        logging.error(f"{funcName!r} does not pass the test")
        return False
      idx -= 1

if __name__ == "__main__":
  parser = argparse.ArgumentParser()
  parser.add_argument("exe", metavar = "executable-path")
  args = parser.parse_args()

  app = os.path.abspath(args.exe)

  LoadTmpDir()
  try:
    ret = main()
  finally:
    DeleteTmpDir()
  sys.exit(0 if ret else 1)
