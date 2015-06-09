#include <inc/lib.h>

int flag;
int recur;

void ssdir(const char*, const char*);
void ss1(const char*, bool, off_t, const char*, const char*);
bool is_snapshot(const char*);

void
snapshot(const char *path, const char *ts)
{
	int r;
	struct Stat st;

	if ((r = stat(path, &st)) < 0)
		panic("stat %s: %e", path, r);
	if (st.st_isdir)
		ssdir(path, ts);
	else
		ss1(0, st.st_isdir, st.st_size, path, ts);
}

void
ssdir(const char *path, const char *ts)
{
	int fd, n;
	struct File f;
	char buf[256];

	if ((fd = open(path, O_RDONLY)) < 0)
		panic("open %s: %e", path, fd);
	while ((n = readn(fd, &f, sizeof f)) == sizeof f)
		if (f.f_name[0] && !is_snapshot(f.f_name[0])) {
			ss1(path, f.f_type==FTYPE_DIR, f.f_size, f.f_name);
			if (recur && f.f_type == FTYPE_DIR) {
				strcpy(buf, path);
				strcat(buf, f.f_name);
				ssdir(buf, ts);
			}
		}
	if (n > 0)
		panic("short read in directory %s", path);
	if (n < 0)
		panic("error reading directory %s: %e", path, n);
}

void
ss1(const char *prefix, bool isdir, off_t size, const char *name)
{
	
}

bool is_snapshot(const char *name)
{
	char *pos = strchr(name, '@');
	if (pos == NULL)
		return false;
	while (pos) {
		if (!isdigit(*pos))
			return false;
		pos++;
	}
	return true;
}

void
usage(void)
{
	printf("usage: snapshot [-(n|c|i)r] [file...]\n");
	exit();
}

void
umain(int argc, char **argv)
{
	int i;
	struct Argstate args;
	char ts[32];

	itoa(sys_get_time(), ts, 10);

	flag = 'i';
	recur = 0;

	argstart(&argc, argv, &args);
	while ((i = argnext(&args)) >= 0)
		switch (i) {
		case 'n':
		case 'c':
		case 'i':
			flag = i;
			break;
		case 'r':
			recur = 1;
		default:
			usage();
		}

	if (argc == 1) 
		snapshot("/", ts);
	else {
		for (i = 1; i < argc; i++)
			snapshot(argv[i], ts);
	}
}