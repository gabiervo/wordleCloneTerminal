import string
import subprocess

l = string.ascii_lowercase

for i in l:
    subprocess.Popen('touch ' + i + ".txt", shell=True)

