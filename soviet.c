// gcc soviet.c proclib.c -I. -g3 && ./a.out mount

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <proclib.h>
#include <arraylib.h>
#include <ctype.h>

char* getFullPath(const char* binname) {
	static const char* dirs[] = { "/usr/bin/", "/bin/", "/sbin/", "/usr/sbin/", "/usr/games/" };

	for (int i = 0; i < ARRAY_SIZE(dirs); i++) {
		static char fullpath[PATH_MAX];
		strcpy(fullpath, dirs[i]);
		strlcat(fullpath, binname, sizeof(fullpath));
		if (access(fullpath, F_OK) == 0)
			return fullpath;
	}

	return "";
}


char* getPkgByFullPath(const char* fullpath) {
	static char pkg[512];

	if (get1LineFromPipe((const char*[]){"/usr/bin/rpm", "-qf", fullpath, NULL}, pkg, sizeof(pkg)) < 0)
		return "";

	return pkg;
}


char* getSrcPkgByPkg(const char* pkg) {
	static char srcPkg[512];

	if (get1LineFromPipe((const char*[]){"/usr/bin/rpm", "-q", "--qf", "%{sourcerpm}", pkg, NULL}, srcPkg, sizeof(srcPkg)) < 0)
		return "";

	return srcPkg;
}


char* getRepoBySrcPkg(const char* srcpkg) {
	for (;;) {
		const char* altPtr = strstr(srcpkg, "-alt"); // можно искать не первое вхождение, а последнее
		if (NULL == altPtr)
			return "";

		if (!isdigit(altPtr[4 /* strlen("-alt") */])){ /* тут еще можно проверить на точку после цифр*/
			srcpkg = altPtr + 4;
			continue; /*на случай, если -alt будет частью имени*/
		}

		for (const char* repoNameEnd = altPtr - 1; repoNameEnd > srcpkg; repoNameEnd--)
			if ('-' == *repoNameEnd) {
				static char repo[512] = "https://git.altlinux.org/gears/X/";
				enum { XPosInRepoStr = 31};
				repo[XPosInRepoStr] = srcpkg[0];
				strncpy(repo+XPosInRepoStr+2, srcpkg, repoNameEnd - srcpkg);
				strcat(repo+XPosInRepoStr+2, ".git");
				return repo;
			}

		return "";
	}
}


int main(int argc, char** argv) {
	if (argc != 1+1) {
		fprintf(stderr, "USAGE: %s <executable_file_name>\n", argv[0]);
		return 1;
	}

	const char* fullPath = getFullPath(argv[1]);
	puts(fullPath);
	const char* pkg = getPkgByFullPath(fullPath);
	puts(pkg);
	const char* srcPkg = getSrcPkgByPkg(pkg);
	puts(srcPkg);
	puts(getRepoBySrcPkg(srcPkg));
	return 0;
}
