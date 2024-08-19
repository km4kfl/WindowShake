# WindowShake

This program collects windows as you use them and adds them to a list. The windows
of this list are shook. By shook they are randomly moved about at a certain interval. The
collection of new windows to be shook can be paused or unpaused with a button.

# Purpose

This tool was intended to supplement the HDCPEnabler tool such that an adversary could
not determine the precise location of a window and thus the content. This would make
the adversary have a difficult time revealing the stream cipher used by HDCP.

It was also intended to be used without HDCP to prevent averaging of frames by an
adversary using software such as TempestSDR. In this case, the moving window causes
the average to either perform worse or to fail.