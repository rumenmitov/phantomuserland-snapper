#include <sys/utsname.h>

char board_name[] = "Genode board";
char arch_name[] = "genode-amd64";

const char *SVN_Version = "<NOT DEFINED>";
const char *svn_version(void)
{
	return SVN_Version;
}

struct utsname phantom_uname = {"Phantom OS",
								"?",
								"?",
								"Genode",
								//	"x86",
								"Genode-amd64"};
