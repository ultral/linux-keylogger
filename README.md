# syscall-interception-x86_64

The module was created as a part of my master thesis in the 2010 year. The master thesis theme is *Keylogging in Linux kernel*. The main idea was to find out a way to intercept system calls for x64 arch Linux kernel, especially, for kernel 2.6.34.7-61.fc13.x86_64. You can read [the article (EN)](http://www.goncharov.xyz/it/system-call-interception-in-linux-kernel-module.html) or [Russian version](https://habr.com/post/110369) about it.

On one hand, it should enough to deal with the system call interception, but on the other hand, I don't sure that nothing has been changed since 2010. So use it **as is**.  Also I created [the demo video](https://www.youtube.com/watch?v=FgPVCQa0qsw).

# Credits
I used code from [vlogger 1.0](https://github.com/vanhauser-thc/THC-Archive) in this work:
