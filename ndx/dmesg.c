/*
 * dmesg.c -- Print out the contents of the kernel ring buffer
 *
 * Copyright (C) 1993 Theodore Ts'o <tytso@athena.mit.edu>
 * Copyright (C) 2011 Karel Zak <kzak@redhat.com>
 *
 * This program comes with ABSOLUTELY NO WARRANTY.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include<winsock.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <windows.h>
#include "dbgexts.h"
#include <wdbgexts.h>
#include <stdlib.h>
#include "dmesg.h"
#include "ndx.h"

/* Close the log.  Currently a NOP. */
#define SYSLOG_ACTION_CLOSE          0
/* Open the log. Currently a NOP. */
#define SYSLOG_ACTION_OPEN           1
/* Read from the log. */
#define SYSLOG_ACTION_READ           2
/* Read all messages remaining in the ring buffer. (allowed for non-root) */
#define SYSLOG_ACTION_READ_ALL       3
/* Read and clear all messages remaining in the ring buffer */
#define SYSLOG_ACTION_READ_CLEAR     4
/* Clear ring buffer. */
#define SYSLOG_ACTION_CLEAR          5
/* Disable printk's to console */
#define SYSLOG_ACTION_CONSOLE_OFF    6
/* Enable printk's to console */
#define SYSLOG_ACTION_CONSOLE_ON     7
/* Set level of messages printed to console */
#define SYSLOG_ACTION_CONSOLE_LEVEL  8
/* Return number of unread characters in the log buffer */
#define SYSLOG_ACTION_SIZE_UNREAD    9
/* Return size of the log buffer */
#define SYSLOG_ACTION_SIZE_BUFFER   10
#define LOG_LEVEL(v) ((v) & 0x07)
#define LX_LOG_FACILITY(v) ((v) >> 3 & 0xff)
#define from_hex(c)		(isdigit(c) ? c - '0' : tolower(c) - 'a' + 10)
#define PRId64       "lld"
# define NBBY            CHAR_BIT
# define isset(a,i)	((a)[(i)/NBBY] & (1<<((i)%NBBY)))
#define _O_RDONLY      0x0000  // open for reading only
#define O_RDONLY     _O_RDONLY
# define __must_be_array(a)	0
# define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]) + __must_be_array(arr))
# define SEEK_DATA	3
#define assert(expression) (void)(                                                       \
            (!!(expression)) ||                                                              \
            (_wassert(_CRT_WIDE(#expression), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)), 0) \
        )


#ifdef ENABLE_NLS
# include <libintl.h>
/*
 * For NLS support in the public shared libraries we have to specify text
 * domain name to be independent on the main program. For this purpose define
 * UL_TEXTDOMAIN_EXPLICIT before you include nls.h to your shared library code.
 */
# ifdef UL_TEXTDOMAIN_EXPLICIT
#  define _(Text) dgettext (UL_TEXTDOMAIN_EXPLICIT, Text)
# else
#  define _(Text) gettext (Text)
# endif
# ifdef gettext_noop
#  define N_(String) gettext_noop (String)
# else
#  define N_(String) (String)
# endif
# define P_(Singular, Plural, n) ngettext (Singular, Plural, n)
#else
# undef bindtextdomain
# define bindtextdomain(Domain, Directory) /* empty */
# undef textdomain
# define textdomain(Domain) /* empty */
# define _(Text) (Text)
# define N_(Text) (Text)
# define P_(Singular, Plural, n) ((n) == 1 ? (Singular) : (Plural))
#endif /* ENABLE_NLS */

uint32_t dmesg_flags = 0;

	struct _stat buf;
	enum {
		ISO_DATE = (1 << 0),
		ISO_TIME = (1 << 1),
		ISO_TIMEZONE = (1 << 2),
		ISO_DOTUSEC = (1 << 3),
		ISO_COMMAUSEC = (1 << 4),
		ISO_T = (1 << 5),
		ISO_GMTIME = (1 << 6),
		ISO_TIMESTAMP = ISO_DATE | ISO_TIME | ISO_TIMEZONE,
		ISO_TIMESTAMP_T = ISO_TIMESTAMP | ISO_T,
		ISO_TIMESTAMP_DOT = ISO_TIMESTAMP | ISO_DOTUSEC,
		ISO_TIMESTAMP_DOT_T = ISO_TIMESTAMP_DOT | ISO_T,
		ISO_TIMESTAMP_COMMA = ISO_TIMESTAMP | ISO_COMMAUSEC,
		ISO_TIMESTAMP_COMMA_T = ISO_TIMESTAMP_COMMA | ISO_T,
		ISO_TIMESTAMP_COMMA_G = ISO_TIMESTAMP_COMMA | ISO_GMTIME,
		ISO_TIMESTAMP_COMMA_GT = ISO_TIMESTAMP_COMMA_G | ISO_T
	};

	struct ul_jsonwrt {
		FILE* out;
		int indent;

		unsigned int after_close : 1;
	};

	char* xstrdup(const char* str)
	{
		char* ret;

		// assert(str);
		ret = _strdup(str);
		if (!ret)
			perror("cannot duplicate string");
		//err(XALLOC_EXIT_CODE, "cannot duplicate string");
		return ret;
	}

	size_t unhexmangle_to_buffer(const char* s, char* buf, size_t len)
	{
		size_t sz = 0;
		const char* buf0 = buf;

		if (!s)
			return 0;

		while (*s && sz < len - 1) {
			if (*s == '\\' && sz + 3 < len - 1 && s[1] == 'x' &&
				isxdigit(s[2]) && isxdigit(s[3])) {

				*buf++ = from_hex(s[2]) << 4 | from_hex(s[3]);
				s += 4;
				sz += 4;
			}
			else {
				*buf++ = *s++;
				sz++;
			}
		}
		*buf = '\0';
		return buf - buf0 + 1;
	}

	static int format_iso_time(struct tm* tm, time_t usec, int flags, char* buf, size_t bufsz)//suseconds_t time_t?
	{
		char* p = buf;
		int len;

		if (flags & ISO_DATE) {
			len = snprintf(p, bufsz, "%4ld-%.2d-%.2d",
				tm->tm_year + (long)1900,
				tm->tm_mon + 1, tm->tm_mday);
			if (len < 0 || (size_t)len > bufsz)
				goto err;
			bufsz -= len;
			p += len;
		}

		if ((flags & ISO_DATE) && (flags & ISO_TIME)) {
			if (bufsz < 1)
				goto err;
			*p++ = (flags & ISO_T) ? 'T' : ' ';
			bufsz--;
		}

		if (flags & ISO_TIME) {
			len = snprintf(p, bufsz, "%02d:%02d:%02d", tm->tm_hour,
				tm->tm_min, tm->tm_sec);
			if (len < 0 || (size_t)len > bufsz)
				goto err;
			bufsz -= len;
			p += len;
		}

		if (flags & ISO_DOTUSEC) {
			len = snprintf(p, bufsz, ".%06"PRId64, (int64_t)usec);
			if (len < 0 || (size_t)len > bufsz)
				goto err;
			bufsz -= len;
			p += len;

		}
		else if (flags & ISO_COMMAUSEC) {
			len = snprintf(p, bufsz, ",%06"PRId64, (int64_t)usec);
			if (len < 0 || (size_t)len > bufsz)
				goto err;
			bufsz -= len;
			p += len;
		}

		return 0;
	err:
		printf(_("format_iso_time: buffer overflow."));//warnx
		return -1;
	}

	int strtimeval_iso(struct timeval* tv, int flags, char* buf, size_t bufsz)
	{
		struct tm tm;
		struct tm* rc;

		if (flags & ISO_GMTIME)
			rc = gmtime_s( &tm ,&tv->tv_sec);
		else
			rc = localtime_s(&tm ,&tv->tv_sec);

		if (rc)
			return format_iso_time(&tm, tv->tv_usec, flags, buf, bufsz);

		printf(_("time %"PRId64" is out of range."), (int64_t)(tv->tv_sec));
		return -1;
	}
struct dmesg_color {
	const char *scheme;	/* name used in termina-colors.d/dmesg.scheme */
	const char *dflt;	/* default color ESC sequence */
};

enum {
	DMESG_COLOR_SUBSYS,
	DMESG_COLOR_TIME,
	DMESG_COLOR_TIMEBREAK,
	DMESG_COLOR_ALERT,
	DMESG_COLOR_CRIT,
	DMESG_COLOR_ERR,
	DMESG_COLOR_WARN,
	DMESG_COLOR_SEGFAULT
};
/*
 * Priority and facility names
 */
struct dmesg_name {
	const char *name;
	const char *help;
};

/*
 * Priority names -- based on sys/syslog.h
 */
static const struct dmesg_name level_names[] =
{
/* [LOG_EMERG] = */{"emerg", "system is unusable"},
/*	[LOG_ALERT]   = */{ "alert", "action must be taken immediately" },
/*[LOG_CRIT] = */{ "crit",  "critical conditions" },
/*[LOG_ERR] = */{"err",   "error conditions"},
/*[LOG_WARNING] = */{"warn",  "warning conditions"},
/*[LOG_NOTICE] = */{"notice","normal but significant condition"},
/*[LOG_INFO] = */{"info",  "informational"},
	/*[LOG_DEBUG] = */{"debug", "debug-level messages"}
};

/*
 * sys/syslog.h uses (f << 3) for all facility codes.
 * We want to use the codes as array indexes, so shift back...
 *
 * Note that libc LOG_FAC() macro returns the base codes, not the
 * shifted code :-)
 */
#define FAC_BASE(f)	((f) >> 3)

static const struct dmesg_name facility_names[] =
{
	/* [FAC_BASE(LOG_KERN)] = */{"kern",     N_("kernel messages")},
	/*[FAC_BASE(LOG_USER)] = */{"user",     N_("random user-level messages")},
	/*[FAC_BASE(LOG_MAIL)] = */{"mail",     N_("mail system")},
	/*[FAC_BASE(LOG_DAEMON)] = */{"daemon",   N_("system daemons")},
	/*[FAC_BASE(LOG_AUTH)] = */{"auth",     N_("security/authorization messages")},
	/*[FAC_BASE(LOG_SYSLOG)] = */{"syslog",   N_("messages generated internally by syslogd")},
	/*[FAC_BASE(LOG_LPR)] = */{"lpr",      N_("line printer subsystem")},
	/*[FAC_BASE(LOG_NEWS)] = */{"news",     N_("network news subsystem")},
	/*[FAC_BASE(LOG_UUCP)] = */{"uucp",     N_("UUCP subsystem")},
	/*[FAC_BASE(LOG_CRON)] = */{"cron",     N_("clock daemon")},
	/*[FAC_BASE(LOG_AUTHPRIV)] = */{"authpriv", N_("security/authorization messages (private)")},
	/*[FAC_BASE(LOG_FTP)] = */{"ftp",      N_("FTP daemon")},
};
/* supported methods to read message buffer
 */
enum {
	DMESG_METHOD_KMSG,	/* read messages from /dev/kmsg (default) */
	DMESG_METHOD_SYSLOG,	/* klogctl() buffer */
	DMESG_METHOD_MMAP	/* mmap file with records (see --file) */
};

enum {
	DMESG_TIMEFTM_NONE = 0,
	DMESG_TIMEFTM_CTIME,		/* [ctime] */
	DMESG_TIMEFTM_CTIME_DELTA,	/* [ctime <delta>] */
	DMESG_TIMEFTM_DELTA,		/* [<delta>] */
	DMESG_TIMEFTM_RELTIME,		/* [relative] */
	DMESG_TIMEFTM_TIME,		/* [time] */
	DMESG_TIMEFTM_TIME_DELTA,	/* [time <delta>] */
	DMESG_TIMEFTM_ISO8601		/* 2013-06-13T22:11:00,123456+0100 */
};
#define is_timefmt(c, f) ((c)->time_fmt == (DMESG_TIMEFTM_ ##f))

struct dmesg_control {
	/* bit arrays -- see include/bitops.h */
	char levels[ARRAY_SIZE(level_names) / NBBY + 1];
	char facilities[ARRAY_SIZE(facility_names) / NBBY + 1];

	struct timeval	lasttime;	/* last printed timestamp */
	struct tm	lasttm;		/* last localtime */
	struct timeval	boot_time;	/* system boot time */
	time_t		suspended_time;	/* time spent in suspended state */

	int		action;		/* SYSLOG_ACTION_* */
	int		method;		/* DMESG_METHOD_* */

	size_t		bufsize;	/* size of syslog buffer */

	int		kmsg;		/* /dev/kmsg file descriptor */
	size_t		kmsg_first_read;/* initial read() return code */
	char		kmsg_buf[BUFSIZ];/* buffer to read kmsg data */

	time_t		since;		/* filter records by time */
	time_t		until;		/* filter records by time */

	/*
	 * For the --file option we mmap whole file. The unnecessary (already
	 * printed) pages are always unmapped. The result is that we have in
	 * memory only the currently used page(s).
	 */
	char		*filename;
	char		*mmap_buff;
	size_t		pagesize;
	unsigned int	time_fmt;	/* time format */

	struct ul_jsonwrt jfmt;		/* -J formatting */

	unsigned int	follow:1,	/* wait for new messages */
			end:1,		/* seek to the of buffer */
			raw:1,		/* raw mode */
			noesc:1,	/* no escape */
			fltr_lev:1,	/* filter out by levels[] */
			fltr_fac:1,	/* filter out by facilities[] */
			decode:1,	/* use "facility: level: " prefix */
			pager:1,	/* pipe output into a pager */
			color:1,	/* colorize messages */
			json:1,		/* JSON output */
			force_prefix:1;	/* force timestamp and decode prefix
					   on each line */
	int		indent;		/* due to timestamps if newline */
};

struct dmesg_record {
	const char	*mesg;
	size_t		mesg_size;
	int     text_length; // added by yuqing
	int		level;
	int		facility;
	struct timeval  tv;

	const char	*next;		/* buffer with next unparsed record */
	size_t		next_size;	/* size of the next buffer */
};

#define INIT_DMESG_RECORD(_r)  do { \
		(_r)->mesg = NULL; \
		(_r)->mesg_size = 0; \
		(_r)->facility = -1; \
		(_r)->level = -1; \
		(_r)->tv.tv_sec = 0; \
		(_r)->tv.tv_usec = 0; \
	} while (0)

static int read_kmsg(struct dmesg_control *ctl);
/*
 * LEVEL     ::= <number> | <name>
 *  <number> ::= @len is set:  number in range <0..N>, where N < ARRAY_SIZE(level_names)
 *           ::= @len not set: number in range <1..N>, where N <= ARRAY_SIZE(level_names)
 *  <name>   ::= case-insensitive text
 *
 *  Note that @len argument is not set when parsing "-n <level>" command line
 *  option. The console_level is interpreted as "log level less than the value".
 *
 *  For example "dmesg -n 8" or "dmesg -n debug" enables debug console log
 *  level by klogctl(SYSLOG_ACTION_CONSOLE_LEVEL, NULL, 8). The @str argument
 *  has to be parsed to number in range <1..8>.
 */
/*
 * FACILITY  ::= <number> | <name>
 *  <number> ::= number in range <0..N>, where N < ARRAY_SIZE(facility_names)
 *  <name>   ::= case-insensitive text
 */
static int parse_facility(const char* str, size_t len)
{
	if (!str)
		return -1;
	if (!len)
		len = strlen(str);
	errno = 0;

	if (isdigit(*str)) {
		char *end = NULL;
		long x = strtol(str, &end, 10);

		if (!errno && end && end > str && (size_t) (end - str) == len &&
		    x >= 0 && (size_t) x < ARRAY_SIZE(facility_names))
			return x;
	} else {
		size_t i;

		for (i = 0; i < ARRAY_SIZE(facility_names); i++) {
			const char *n = facility_names[i].name;

			if (strncasecmp(str, n, len) == 0 && *(n + len) == '\0')
				return i;
		}
	}

	if (errno)
		err(EXIT_FAILURE, _("failed to parse facility '%s'"), str);

	errx(EXIT_FAILURE, _("unknown facility '%s'"), str);
	return -1;
}
/*
 * Parses numerical prefix used for all messages in kernel ring buffer.
 *
 * Priorities/facilities are encoded into a single 32-bit quantity, where the
 * bottom 3 bits are the priority (0-7) and the top 28 bits are the facility
 * (0-big number).
 *
 * Note that the number has to end with '>' or ',' char.
 */
static const char* parse_faclev(const char* str, int* fac, int* lev)
{
	long num;
	char *end = NULL;

	if (!str)
		return str;

	errno = 0;
	num = strtol(str, &end, 10);

	if (!errno && end && end > str) {
		*fac = LX_LOG_FACILITY(num);
		*lev = LOG_LEVEL(num);

		if (*lev < 0 || (size_t) *lev > ARRAY_SIZE(level_names))
			*lev = -1;
		if (*fac < 0 || (size_t) *fac > ARRAY_SIZE(facility_names))
			*fac = -1;
		return end + 1;		/* skip '<' or ',' */
	}

	return str;
}
/*
 * Parses timestamp from syslog message prefix, expected format:
 *
 *	seconds.microseconds]
 *
 * the ']' is the timestamp field terminator.
 */
static const char *parse_syslog_timestamp(const char *str0, struct timeval *tv)
{
	const char *str = str0;
	char *end = NULL;

	if (!str0)
		return str0;

	errno = 0;
	tv->tv_sec = strtol(str, &end, 10);

	if (!errno && end && *end == '.' && *(end + 1)) {
		str = end + 1;
		end = NULL;
		tv->tv_usec = strtol(str, &end, 10);
	}
	if (errno || !end || end == str || *end != ']')
		return str0;

	return end + 1;	/* skip ']' */
}
/*
 * Parses timestamp from /dev/kmsg, expected formats:
 *
 *	microseconds,
 *	microseconds;
 *
 * the ',' is fields separators and ';' items terminator (for the last item)
 */
static const char *parse_kmsg_timestamp(const char *str0, struct timeval *tv)
{
	const char *str = str0;
	char *end = NULL;
	uint64_t usec;

	if (!str0)
		return str0;

	errno = 0;
	usec = strtoumax(str, &end, 10);

	if (!errno && end && (*end == ';' || *end == ',')) {
		tv->tv_usec = usec % 1000000;
		tv->tv_sec = usec / 1000000;
	} else
		return str0;

	return end + 1;	/* skip separator */
}

static double time_diff(struct timeval *a, struct timeval *b)
{
	return (a->tv_sec - b->tv_sec) + (a->tv_usec - b->tv_usec) / 1E6;
}

static int get_syslog_buffer_size(void)
{
	int n = klogctl(SYSLOG_ACTION_SIZE_BUFFER, NULL, 0);

	return n > 0 ? n : 0;
}
/*
 * Reads messages from regular file by mmap
 */
/*
 * Reads messages from kernel ring buffer by klogctl()
 */
static size_t read_buffer(struct dmesg_control *ctl, char **buf)
{
	size_t n = -1;

	switch (ctl->method) {
	case DMESG_METHOD_MMAP:
		n = mmap_file_buffer(ctl, buf);
		break;

	case DMESG_METHOD_KMSG:
		/*
		 * Since kernel 3.5.0
		 */
		n = read_kmsg(ctl);
		break;
	default:
		abort();	/* impossible method -> drop core */
	}

	return n;
}

static int fwrite_hex(const char *buf, size_t size, FILE *out)
{
	size_t i;

    for (i = 0; i < size; i++) {
		int rc = fprintf(out, "\\x%02hhx  ", buf[i]);
		if (rc < 0)
			return rc;
	}
	
	return 0;
}

/*
 * Prints to 'out' and non-printable chars are replaced with \x<hex> sequences.
 */
static void safe_fwrite(struct dmesg_control *ctl, const char *buf, size_t size, int indent, FILE *out)
{
	size_t i;

	for (i = 0; i < size; i++) {
		const char *p = buf + i;
		int rc, hex = 0;
		size_t len = 1;

		if (!ctl->noesc) {
			if (*p == '\0') {
				hex = 1;
				goto doprint;
			}
			{
				len = 1;
				if (!isprint((unsigned char) *p) &&
				    !isspace((unsigned char) *p))        /* non-printable */
					hex = 1;
			}
		}

doprint:
		if (hex)
			rc = fwrite_hex(p, len, out);
		else if (*p == '\n' && *(p + 1) && indent) 
		{
		    rc = fwrite(p, 1, len, out) != len;
			if (fprintf(out, "%*s ", indent, "\n") != indent)
				rc |= 1;
		} else
			rc = fwrite(p, 1, len, out) != len;
	}
}

static const char *skip_item(const char *begin, const char *end, const char *sep)
{
	while (begin < end) {
		int c = *begin++;

		if (c == '\0' || strchr(sep, c))
			break;
	}

	return begin;
}

/*
 * Parses one record from syslog(2) buffer
 */
static int get_next_syslog_record(struct dmesg_control *ctl,
				  struct dmesg_record *rec)
{
	size_t i;
	const char *begin = NULL;

	if (ctl->method != DMESG_METHOD_MMAP &&
	    ctl->method != DMESG_METHOD_SYSLOG)
		return -1;

	if (!rec->next || !rec->next_size)
		return 1;

	INIT_DMESG_RECORD(rec);

	/*
	 * Unmap already printed file data from memory
	 */
	if (ctl->mmap_buff && (size_t) (rec->next - ctl->mmap_buff) > ctl->pagesize) {
		void *x = ctl->mmap_buff;

		ctl->mmap_buff += ctl->pagesize;
		//munmap(x, ctl->pagesize);
		//解除映射地址

	}

	for (i = 0; i < rec->next_size; i++) {
		const char *p = rec->next + i;
		const char *end = NULL;

		if (!begin)
			begin = p;
		if (i + 1 == rec->next_size) {
			end = p + 1;
			i++;
		} else if (*p == '\n' && *(p + 1) == '<')
			end = p;

		if (begin && !*begin)
			begin = NULL;	/* zero(s) at the end of the buffer? */
		if (!begin || !end)
			continue;
		if (end <= begin)
			continue;	/* error or empty line? */

		if (*begin == '<') {
			if (ctl->fltr_lev || ctl->fltr_fac || ctl->decode || ctl->color || ctl->json)
				begin = parse_faclev(begin + 1, &rec->facility,
						     &rec->level);
			else
				begin = skip_item(begin, end, ">");
		}

		if (*begin == '[' && (*(begin + 1) == ' ' ||
				      isdigit(*(begin + 1)))) {

			if (!is_timefmt(ctl, NONE))
				begin = parse_syslog_timestamp(begin + 1, &rec->tv);
			else
				begin = skip_item(begin, end, "]");

			if (begin < end && *begin == ' ')
				begin++;
		}

		rec->mesg = begin;
		rec->mesg_size = end - begin;
		/* Don't count \n from the last message to the message size */
		if (*end != '\n' && *(end - 1) == '\n')
			rec->mesg_size--;

		rec->next_size -= end - rec->next;
		rec->next = rec->next_size > 0 ? end + 1 : NULL;
		if (rec->next_size > 0)
			rec->next_size--;

		return 0;
	}

	return 1;
}

static time_t record_time(struct dmesg_control *ctl, struct dmesg_record *rec)
{
	return ctl->boot_time.tv_sec + ctl->suspended_time + rec->tv.tv_sec;
}

static int accept_record(struct dmesg_control *ctl, struct dmesg_record *rec)
{
	if (ctl->fltr_lev && (rec->facility < 0 ||
			      !isset(ctl->levels, rec->level)))
		return 0;

	if (ctl->fltr_fac && (rec->facility < 0 ||
			      !isset(ctl->facilities, rec->facility)))
		return 0;

	if (ctl->since && ctl->since >= record_time(ctl, rec))
		return 0;

	if (ctl->until && ctl->until <= record_time(ctl, rec))
		return 0;

	return 1;
}

static void raw_print(struct dmesg_control *ctl, const char *buf, size_t size)
{
	int lastc ='\n';

	if (!ctl->mmap_buff) {
		/*
		 * Print whole ring buffer
		 */
		safe_fwrite(ctl, buf, size, 0, stdout);
		lastc = buf[size - 1];
	} else {
		/*
		 * Print file in small chunks to save memory
		 */
		while (size) {
			size_t sz = size > ctl->pagesize ? ctl->pagesize : size;
			char *x = ctl->mmap_buff;

			safe_fwrite(ctl, x, sz, 0, stdout);
			lastc = x[sz - 1];
			size -= sz;
			ctl->mmap_buff += sz;
			//munmap(x, sz);
		}
	}

	if (lastc !='\n')
		dprintf("\n");
}

static struct tm *record_localtime(struct dmesg_control *ctl,
				   struct dmesg_record *rec,
				   struct tm *tm)
{
	time_t t = record_time(ctl, rec);
	return localtime_s(tm,&t);
}

static char *record_ctime(struct dmesg_control *ctl,
			  struct dmesg_record *rec,
			  char *buf, size_t bufsiz)
{
	struct tm tm;

	record_localtime(ctl, rec, &tm);

	/* TRANSLATORS: dmesg uses strftime() fo generate date-time string
	   where %a is abbreviated name of the day, %b is abbreviated month
	   name and %e is day of the month as a decimal number. Please, set
	   proper month/day order here */
	if (strftime(buf, bufsiz, _("%a %b %e %H:%M:%S %Y"), &tm) == 0)
		*buf = '\0';
	return buf;
}

static char *short_ctime(struct tm *tm, char *buf, size_t bufsiz)
{
	/* TRANSLATORS: dmesg uses strftime() fo generate date-time string
	   where: %b is abbreviated month and %e is day of the month as a
	   decimal number. Please, set proper month/day order here. */
	if (strftime(buf, bufsiz, _("%b%e %H:%M"), tm) == 0)
		*buf = '\0';
	return buf;
}

static char *iso_8601_time(struct dmesg_control *ctl, struct dmesg_record *rec,
			   char *buf, size_t bufsz)
{
	struct timeval tv = {
		.tv_sec = ctl->boot_time.tv_sec + ctl->suspended_time + rec->tv.tv_sec,
		.tv_usec = rec->tv.tv_usec
	};

	if (strtimeval_iso(&tv,	ISO_TIMESTAMP_COMMA_T, buf, bufsz) != 0)
		return NULL;

	return buf;
}

static double record_count_delta(struct dmesg_control *ctl,
				 struct dmesg_record *rec)
{
	double delta = 0;

	if (timerisset(&ctl->lasttime))
		delta = time_diff(&rec->tv, &ctl->lasttime);

	ctl->lasttime = rec->tv;
	return delta;
}

static const char *get_subsys_delimiter(const char *mesg, size_t mesg_size)
{
	const char *p = mesg;
	size_t sz = mesg_size;

	while (sz > 0) {
		const char *d = strnchr(p, sz, ':');
		if (!d)
			return NULL;
		sz -= d - p + 1;
		if (sz) {
			if (sz >= 2 && isblank(*(d + 1)))
				return d + 2;
			p = d + 1;
		}
	}
	return NULL;
}
#define is_facpri_valid(_r)	\
	    (((_r)->level > -1) && ((_r)->level < (int) ARRAY_SIZE(level_names)) && \
	     ((_r)->facility > -1) && \
	     ((_r)->facility < (int) ARRAY_SIZE(facility_names)))
static void print_record(struct dmesg_control *ctl,
			 struct dmesg_record *rec)
{
	char buf[128];
	char fpbuf[32] = "\0";
	char tsbuf[64] = "\0";
	size_t mesg_size = rec->mesg_size;
	int timebreak = 0;
	char *mesg_copy = NULL;
	const char *line = NULL;
	if (!accept_record(ctl, rec))
		return;

	if (!rec->mesg_size) {
		if (!ctl->json)
			dprintf("\n");
		return;
	}
	/*
	 * Compose syslog(2) compatible raw output -- used for /dev/kmsg for
	 * backward compatibility with syslog(2) buffers only
	 */
	if (ctl->raw) {
		ctl->indent = snprintf(tsbuf, sizeof(tsbuf),
				       "<%d>[%5ld.%06ld]  ",
			(long)rec->facility,
				      // LOG_MAKEPRI(rec->facility, rec->level),
				       (long) rec->tv.tv_sec,
				       (long) rec->tv.tv_usec);
		goto full_output;
	}
	/* Store decode information (facility & priority level) in a buffer */
	if (!ctl->json && ctl->decode && is_facpri_valid(rec))
		snprintf(fpbuf, sizeof(fpbuf), "%-6s:%-6s: ",
			 facility_names[rec->facility].name,
			 level_names[rec->level].name);
	/* Store the timestamp in a buffer */
	switch (ctl->time_fmt) {
		double delta;
		struct tm cur;
	case DMESG_TIMEFTM_NONE:
		ctl->indent = 0;
		break;
	case DMESG_TIMEFTM_CTIME:
		ctl->indent = snprintf(tsbuf, sizeof(tsbuf), "[%s] ",
				      record_ctime(ctl, rec, buf, sizeof(buf)));
		break;
	case DMESG_TIMEFTM_CTIME_DELTA:
		ctl->indent = snprintf(tsbuf, sizeof(tsbuf), "[%s <%12.06f>] ",
				      record_ctime(ctl, rec, buf, sizeof(buf)),
				      record_count_delta(ctl, rec));
		break;
	case DMESG_TIMEFTM_DELTA:
		ctl->indent = snprintf(tsbuf, sizeof(tsbuf), "[<%12.06f>] ",
				      record_count_delta(ctl, rec));
		break;
	case DMESG_TIMEFTM_RELTIME:
		record_localtime(ctl, rec, &cur);
		delta = record_count_delta(ctl, rec);
		if (cur.tm_min != ctl->lasttm.tm_min ||
		    cur.tm_hour != ctl->lasttm.tm_hour ||
		    cur.tm_yday != ctl->lasttm.tm_yday) {
			timebreak = 1;
			ctl->indent = snprintf(tsbuf, sizeof(tsbuf), "[%s] ",
					      short_ctime(&cur, buf,
							  sizeof(buf)));
		} else {
			if (delta < 10)
				ctl->indent =snprintf(tsbuf, sizeof(tsbuf),
						"[%+8.06f]",  delta);
			else
				ctl->indent = snprintf(tsbuf, sizeof(tsbuf),
						"[%+9.06f]", delta);
		}
		ctl->lasttm = cur;
		break;
	case DMESG_TIMEFTM_TIME:
		ctl->indent = snprintf(tsbuf, sizeof(tsbuf),
				      ctl->json ? "%5ld.%06ld" : "[%5ld.%06ld]",
				      (long)rec->tv.tv_sec,
				      (long)rec->tv.tv_usec);
		break;
	case DMESG_TIMEFTM_TIME_DELTA:
		ctl->indent = snprintf(tsbuf, sizeof(tsbuf), "[%5ld.%06ld <%12.06f>] ",
				      (long)rec->tv.tv_sec,
				      (long)rec->tv.tv_usec,
				      record_count_delta(ctl, rec));
		break;
	case DMESG_TIMEFTM_ISO8601:
		ctl->indent = snprintf(tsbuf, sizeof(tsbuf), "%s ",
				      iso_8601_time(ctl, rec, buf,
						    sizeof(buf)));
		break;
	default:
		abort();
	}

	ctl->indent += strlen(fpbuf);

full_output:
	/* Output the decode information */
	if (*fpbuf) {
		dprintf(fpbuf);
	} 
	/*
	else if (ctl->json && is_facpri_valid(rec)) {
		if (ctl->decode) {
		ul_jsonwrt_value_s(&ctl->jfmt, "fac", facility_names[rec->facility].name);
			ul_jsonwrt_value_s(&ctl->jfmt, "pri", level_names[rec->level].name);
		} else
			ul_jsonwrt_value_u64(&ctl->jfmt, "pri", LOG_MAKEPRI(rec->facility, rec->level));
	}
	*/
	/* Output the timestamp buffer */
	if (*tsbuf) {
		/* Colorize the timestamp */
	//	if (ctl->color)
	//		dmesg_enable_color(timebreak ? DMESG_COLOR_TIMEBREAK :
	//					       DMESG_COLOR_TIME);
		if (ctl->time_fmt != DMESG_TIMEFTM_RELTIME) {
			if (ctl->json)
				dprintf("ul_jsonwrt_value_raw");
			else
				dprintf(tsbuf,stdout);
		}
		else {
			/*
			 * For relative timestamping, the first line's
			 * timestamp is the offset and all other lines will
			 * report an offset of 0.000000.
			 */
			dprintf(!line ? tsbuf : "[  +0.000000] ", stdout);
		}
		dprintf("[%s]",level_names[rec->level].name);
	}

	/*
	 * A kernel message may contain several lines of output, separated
	 * by '\n'.  If the timestamp and decode outputs are forced then each
	 * line of the message must be displayed with that information.
	 */
	if (ctl->force_prefix) {
		if (!line) {
			mesg_copy = xstrdup(rec->mesg);
			line = strtok(mesg_copy, "\n");
			if (!line)
				goto done;	/* only when something is wrong */
		}
	} 
	else {
		line = rec->mesg;
		mesg_size = rec->mesg_size;
	}

#ifdef HAS_COLOR
	/* Colorize kernel message output */
	if (ctl->color) {
		/* Subsystem prefix */
		const char *subsys = get_subsys_delimiter(line, mesg_size);
		int has_color = 0;

		if (subsys) {
			dmesg_enable_color(DMESG_COLOR_SUBSYS);
			safe_fwrite(ctl, line, subsys - line, ctl->indent, stdout);
			color_disable();

			mesg_size -= subsys - line;
			line = subsys;
		}
		/* Error, alert .. etc. colors */
		//has_color = set_level_color(rec->level, line, mesg_size) == 0;
		safe_fwrite(ctl, line, mesg_size, ctl->indent, stdout);
		if (has_color)
			color_disable();
	} else {
		if (ctl->json)
			ul_jsonwrt_value_s(&ctl->jfmt, "msg", line);
		else
			safe_fwrite(ctl, line, mesg_size, ctl->indent, stdout);
	}
#else
	dprintf(line);
#endif
	/* Get the next line */
	if (ctl->force_prefix) {
		line = strtok(NULL, "\n");
		if (line && *line) {
			dprintf("\n");
			mesg_size = strlen(line);
			goto full_output;
		}
	}

done:
	free(mesg_copy);
	if (ctl->json)
		dprintf("ul_jsonwrt_object_close\n");
	else

		dprintf("\n");
}

/*
 * Prints the 'buf' kernel ring buffer; the messages are filtered out according
 * to 'levels' and 'facilities' bitarrays.
 */
static void print_buffer(struct dmesg_control* ctl,
	const char* buf, size_t size)
{
	size_t sz = size;
	const char* cursor = buf, *end = buf + size;
	struct dmesg_record rec = { .next = buf, .next_size = size };

	if (ctl->raw) {
		raw_print(ctl, buf, size);
		return;
	}
	do {
		if (parse_kmsg_record(ctl, &rec, cursor, (size_t)sz) == 0) {
			print_record(ctl, &rec);
		}
		cursor += rec.mesg_size;
	} while (cursor < end);
}

static size_t read_kmsg_one(struct dmesg_control *ctl)
{
	size_t size;

	/* kmsg returns EPIPE if record was modified while reading */
	do {
		size = read(ctl->kmsg, ctl->kmsg_buf,
			    sizeof(ctl->kmsg_buf) - 1);
	} while (size < 0 && errno == EPIPE);

	return size;
}
//需要，稍后替换//
static int init_kmsg(struct dmesg_control* ctl)
{
	int o_nonblock = 1;
	int mode = O_RDONLY;

	if (!ctl->follow)
		mode |= o_nonblock;
	else
		setlinebuf(stdout);

	ctl->kmsg = open("/dev/kmsg", mode);
	if (ctl->kmsg < 0)
		return -1;

	/*
	 * Seek after the last record available at the time
	 * the last SYSLOG_ACTION_CLEAR was issued.
	 *
	 * ... otherwise SYSLOG_ACTION_CLEAR will have no effect for kmsg.
	 */
	lseek(ctl->kmsg, 0, ctl->end ? SEEK_END : SEEK_DATA);

	/*
	 * Old kernels (<3.5) can successfully open /dev/kmsg for read-only,
	 * but read() returns -EINVAL :-(((
	 *
	 * Let's try to read the first record. The record is later processed in
	 * read_kmsg().
	 */
	ctl->kmsg_first_read = read_kmsg_one(ctl);
	if (ctl->kmsg_first_read < 0) {
		close(ctl->kmsg);
		ctl->kmsg = -1;
		return -1;
	}

	return 0;
}

/*
 * /dev/kmsg record format:
 *
 *     faclev,seqnum,timestamp[optional, ...];message\n
 *      TAGNAME=value
 *      ...
 *
 * - fields are separated by ','
 * - last field is terminated by ';'
 *
 */
#define LAST_KMSG_FIELD(s)	(!s || !*s || *(s - 1) == ';')

static int parse_kmsg_record(struct dmesg_control* ctl,
	struct dmesg_record* rec,
	char* buf,
	size_t sz)
{
	struct printk_log* kmsg = (struct printk_log*)buf;
	rec->facility = kmsg->facility;
	rec->level = kmsg->level;
	rec->mesg = buf + sizeof(struct printk_log);
	rec->mesg_size = kmsg->len;
	rec->text_length = kmsg->text_len;
	rec->tv.tv_usec = kmsg->ts_nsec/1000;
 	*((char*)buf + sizeof(struct printk_log) + kmsg->text_len) = '\0';	/* for debug messages */
	return 0;
}

/*
 * Note that each read() call for /dev/kmsg returns always one record. It means
 * that we don't have to read whole message buffer before the records parsing.
 *
 * So this function does not compose one huge buffer (like read_syslog_buffer())
 * and print_buffer() is unnecessary. All is done in this function.
 *
 * Returns 0 on success, -1 on error.
 */
static int read_kmsg(struct dmesg_control *ctl)
{
	struct dmesg_record rec;
	size_t sz;

	if (ctl->method != DMESG_METHOD_KMSG || ctl->kmsg < 0)
		return -1;

	/*
	 * The very first read() call is done in kmsg_init() where we test
	 * /dev/kmsg usability. The return code from the initial read() is
	 * stored in ctl->kmsg_first_read;
	 */
	sz = ctl->kmsg_first_read;

	while (sz > 0) {
		*(ctl->kmsg_buf + sz) = '\0';	/* for debug messages */

		if (parse_kmsg_record(ctl, &rec,
				      ctl->kmsg_buf, (size_t) sz) == 0)
			print_record(ctl, &rec);

		sz = read_kmsg_one(ctl);
	}

	return 0;
}

static int which_time_format(const char *s)
{
	if (!strcmp(s, "notime"))
		return DMESG_TIMEFTM_NONE;
	if (!strcmp(s, "ctime"))
		return DMESG_TIMEFTM_CTIME;
	if (!strcmp(s, "delta"))
		return DMESG_TIMEFTM_DELTA;
	if (!strcmp(s, "reltime"))
		return DMESG_TIMEFTM_RELTIME;
	if (!strcmp(s, "iso"))
		return DMESG_TIMEFTM_ISO8601;
	errx(EXIT_FAILURE, _("unknown time format: %s"), s);
}

#ifdef TEST_DMESG
static inline int dmesg_get_boot_time(struct timeval *tv)
{
	char *str = getenv("DMESG_TEST_BOOTIME");
	uintmax_t sec, usec;

	if (str && sscanf(str, "%ju.%ju", &sec, &usec) == 2) {
		tv->tv_sec = sec;
		tv->tv_usec = usec;
		return tv->tv_sec >= 0 && tv->tv_usec >= 0 ? 0 : -EINVAL;
	}

	return get_boot_time(tv);
}

static inline time_t dmesg_get_suspended_time(void)
{
	if (getenv("DMESG_TEST_BOOTIME"))
		return 0;
	return get_suspended_time();
}
#else
# define dmesg_get_boot_time	get_boot_time
# define dmesg_get_suspended_time	get_suspended_time
#endif

int main(int argc, char* argv[])
{
	if (argc < 2) {
		printf("missing data file\n");
		return -1;
	}
	return dmesg_decode_file(argv[1]);
}
int dmesg_decode_file(const char* filename)
{
	struct stat sb;
	struct dmesg_control ctl = {
	.filename = filename,
	.action = SYSLOG_ACTION_READ_ALL,
	.method = DMESG_METHOD_KMSG,
	.kmsg = -1,
	.time_fmt = DMESG_TIMEFTM_TIME,
	.indent = 0,
	.raw = 0,
	};
	if (stat(filename, &sb) == -1) {
		dprintf("Failed stat ");
		return;
		//exit(EXIT_FAILURE);//BUG在这里
	}
	dprintf("File size:                %lld bytes\n",(long long)sb.st_size);
	dprintf("%s\n", ctl.filename);

	int fd = open(filename, O_RDONLY);//only read//
	char* p = NULL;
	p = (char*)malloc(sb.st_size);
	int rfd = read(fd,p,sb.st_size);
	if (rfd < 0) {
		return -1;
		dprintf("read file fail\n");
	}
	close(fd);
	
	print_buffer(&ctl, p, sb.st_size);

	if (NULL != p)
	{
		free(p);
	}
	return 0;
}

int dmesg_decode_memory() {};


#define MAX_LOG_LEN 4096

void dump_record(uint64_t desc, uint64_t info, int prev_flags, uint64_t prb_ptr, uint64_t data_ptr) {
	//以下变量原文未详述，需后续查证,猜测为命令行参数。
	//int arg0 = 0;
	//int arg1 = 0;
	//int arg2 = 0;
	//int argc = 0;
	//if (argc > 2)
	//	prev_flags = arg2;
	//else
	//	prev_flags = 0;

	int prefix = 1;
	int newline = 1;
	int text_len = 0;
	uint64_t log = NULL;
	int info_flags = 0;
	uint64_t ts_nsec = 0;
	uint64_t tbl_begin = 0;
	uint64_t tbl_next = 0;
	uint64_t begin = 0;
	uint64_t next = 0;
	uint32_t size_bits = 0;
	ULONG read = 0;
	char log_cache[MAX_LOG_LEN];

	static int32_t text_blk_lpos_offset = -1;
	static int32_t tbl_begin_offset = -1;
	static int32_t tbl_next_offset = -1;
	static int32_t text_data_ring_offset = -1;
	static int32_t data_offset = -1;
	static int32_t subsystem_offset = -1;
	static int32_t device_offset = -1;
	static int32_t dev_info_offset = -1;

	if (text_blk_lpos_offset < 0) {
		text_blk_lpos_offset = NdxGetOffset("prb_desc","text_blk_lpos");
		tbl_begin_offset = NdxGetOffset("prb_data_blk_lpos","begin");
		tbl_next_offset = NdxGetOffset("prb_data_blk_lpos", "next");
		text_data_ring_offset = NdxGetOffset("prb", "text_data_ring");
		data_offset = NdxGetOffset("prb_data_ring", "data");
		subsystem_offset = NdxGetOffset("dev_printk_info","subsystem");
		device_offset = NdxGetOffset("dev_printk_info","device");
		dev_info_offset = NdxGetOffset("printk_info","dev_info");
	}

	uint64_t dev_info_addr = info + dev_info_offset;
	uint64_t data_addr = prb_ptr + text_data_ring_offset + data_offset;
	uint64_t subsystem_addr = dev_info_addr + subsystem_offset;
	uint64_t device_addr = dev_info_addr + device_offset;
	uint64_t tbl_begin_addr = desc+ text_blk_lpos_offset+tbl_begin_offset;
	uint64_t tbl_next_addr = desc+ text_blk_lpos_offset+tbl_next_offset;

	//dprintf("tbl_begin_addr: %p ", tbl_begin_addr, tbl_begin_addr);
	//dprintf("tbl_next_addr: %p\n", tbl_next_addr, tbl_next_addr);

	GetFieldValue(prb_ptr + text_data_ring_offset, "prb_data_ring", "size_bits", size_bits);
	GetFieldValue(info, "printk_info", "ts_nsec", ts_nsec);
	GetFieldValue(info, "printk_info", "flags", info_flags);
	GetFieldValue(tbl_begin_addr,"prb_data_blk_lpos","begin", tbl_begin);
	GetFieldValue(tbl_next_addr-8,"prb_data_blk_lpos","next",tbl_next );

	begin = (tbl_begin%(1U<<size_bits));
	next = (tbl_next%(1U<<size_bits));
	
	//dprintf("(1U << size_bits): %x size_bits: %d ", (1U << size_bits), size_bits);
	//dprintf("begin: %x ", begin);
	//dprintf("next: %x\n", next);

	if (begin & 1) {
		text_len = 0;
		log = "";
	}
	else
	{
		if (begin > next) {
			begin = 0;
		}
		begin = begin + sizeof(long);
		
		int temp_text_len;
		GetFieldValue( info,"printk_info","text_len", temp_text_len);
		//dprintf("text_len: %d ",temp_text_len);
		if (next - begin < temp_text_len)
			text_len = next - begin;
		else
			text_len = temp_text_len;
			
		log = data_ptr + begin;
	}
	if ((prev_flags & 8) && !(info_flags & 4))
		prefix = 0;
	
	if (info_flags & 8) {
		if ((prev_flags & 8) && !(prev_flags & 2))
			prefix = 0;
		if (!(info_flags & 2))
			newline = 0;
	}

	if (prefix)
		//dprintf("ts_nsec: %p\n",ts_nsec);
		dprintf("[%5lu.%06lu] ", ts_nsec / 1000000000 , ts_nsec % 1000000000);
		if (text_len)
		//eval "printf \"%%%d.%ds\", $log", $text_len, $text_len
		{
			
			//logs_cache = (char*)malloc(text_len);
			text_len = (text_len >= sizeof(log_cache)) ? sizeof(log_cache) - 1 : text_len;
			//logs_cache = (char*)malloc(text_len);
			ReadMemory(log,log_cache,text_len+4, &read);
			if (read > 0) {
				log_cache[text_len+4] = 0;
				dprintf("%s", log_cache + 4);
			}
		}
		
		//dprintf("%%%d.%ds", log, text_len,text_len);
	if(dmesg_flags & NDX_DMESG_OPTION_VERBOSE)
		dprintf("--<text_len: %d desc: %p info: %p log_addr: %p>--", text_len, desc, info, log);

	if (newline) {
		dprintf("\n");
	}

	char dict[MAX_LOG_LEN];
	int dict_len = 0;
	//int dict_len = sizeof(*subsystem_ptr);
	//dprintf("subsystem_addr:%p ,device_addr:%p\n", subsystem_addr, device_addr);
	ReadMemory(subsystem_addr, dict, 1, &read);
	if (read > 0&& dict[0] != '\0') {
		dict_len = 16;
		ReadMemory(subsystem_addr, dict, dict_len, &read);
		dprintf("SUBSYSTEM=");
		int idx = 0;
		while (idx < dict_len)
		{
			char c = dict[idx];
			if (c == '\0')
				break;
			else
			{
				if (c < ' ' || c >= 127 || c == '\\')
					dprintf("\\x%02x", c);
				else
					dprintf("%c", c);
			}
			idx = idx + 1;
		}
		dprintf("\n");
	}

	//dict = device_addr;
	//dict_len = sizeof(*device_ptr);
	ReadMemory(device_addr, dict, 1, &read);
	if (read > 0&& dict[0] != '\0') {
		dict_len = 48;
		ReadMemory(device_addr, dict, dict_len, &read);
		dprintf("DEVICE=");
		int idx = 0;
		while (idx < dict_len)
		{
			char c = dict[idx];
			if (c == '\0')
				break;
			else
			{
				if (c < ' ' || c >= 127 || c == '\\')
					dprintf("\\x%02x", c);
				else
					dprintf("%c", c);
			}
			idx = idx + 1;
		}
		dprintf("\n");
	}
}

// ndx_dmesgx(printk_rb_dynamic)
// ndx_dmesgx(printk_rb_static)

#define NDX_DMESG_DESC_COMMITED						(1<<0)
#define NDX_DMESG_DESC_FINALIZED					(1<<1)

int ndx_dmesgx(uint64_t prb_ptr)
{
	int desc_commited = 1;
	int desc_finalized = 2;
	int desc_sv_bits = sizeof(long) * 8;
	int desc_flags_shift = desc_sv_bits - 2;
	int desc_flags_mask = 3U << desc_flags_shift;
	int id_mask = ~desc_flags_mask;
	int prev_flags = 0;

	int64_t id;
	int64_t end_id;
	uint32_t count_bits;
	uint32_t desc_count;
	uint64_t desc_ring_addr = prb_ptr + NdxGetOffset("printk_rb_static", "desc_ring");
	uint64_t tail_id_addr = desc_ring_addr + NdxGetOffset("prb_desc_ring", "tail_id");
	int state_var_offset = NdxGetOffset("prb_desc", "state_var");
	int info_flags_offset = NdxGetOffset("printk_info", "flags");
	int info_flags = 0;
	int64_t state_var_counter = 0;

	GetFieldValue(desc_ring_addr, "prb_desc_ring", "count_bits", count_bits);
	desc_count = 1 << count_bits;
	GetFieldValue(desc_ring_addr, "prb_desc_ring", "tail_id.counter", id);
	GetFieldValue(desc_ring_addr, "prb_desc_ring", "head_id.counter", end_id);
	int sz_desc = GetTypeSize("lk!prb_desc");
	int sz_info = GetTypeSize("lk!printk_info");

	uint32_t text_data_ring_offset = NdxGetOffset("prb", "text_data_ring");
	uint32_t data_offset = NdxGetOffset("prb_data_ring", "data");
	uint64_t data_addr = prb_ptr + text_data_ring_offset + data_offset;
	uint64_t data_ptr = 0;
	char* data =NULL;
	
	ReadPointer(data_addr, &data_ptr);
	ReadPointer(data_addr,&data);
	uint64_t descs_addr = desc_ring_addr + NdxGetOffset("prb_desc_ring", "descs");
	uint64_t infos_addr = desc_ring_addr + NdxGetOffset("prb_desc_ring", "infos");
	uint64_t descs_ptr = 0;
	uint64_t infos_ptr = 0;
	ReadPointer(descs_addr, &descs_ptr);
	ReadPointer(infos_addr, &infos_ptr);


	int i = 0;
	dprintf("Dump of pdb %p data_addr %p\n", prb_ptr, data_addr);

	end_id = end_id & id_mask;

	while (1)
	{
		uint64_t desc = descs_ptr + sz_desc * (id % desc_count);
		uint64_t info = infos_ptr + sz_info * (id % desc_count);		

		GetFieldValue(desc, "prb_desc", "state_var.counter", state_var_counter);
		unsigned long state = 3 & (state_var_counter >> desc_flags_shift);
		if (CheckControlC())
		{
			break;
		}
		if (state & NDX_DMESG_DESC_COMMITED || state & NDX_DMESG_DESC_FINALIZED) {
			dump_record(desc, info, prev_flags, prb_ptr, data_ptr);
			GetFieldValue(info, "printk_info", "flags", info_flags);
			prev_flags = info_flags;
		}
		id = (id + 1) & id_mask;
		if (id == end_id) {
			break;
		}
		i++;
	}
	dprintf("total %d records dumped\n",i);
}

//void NdxDmesgUsage()
//{
//	dprintf("!ready to show linux ready queue or a CPU\n."
//		" Syntax: !ready [-c cpu-no] [-f display-flags]\n"
//		" display flags:\n"
//		"  Bit0: task list on ready queue.\n"
//		"  Bit1: queue attributes.\n"
//	);
//}

/*
dmesg [options]
-f read file,only legcy
-a address of prb, non-legacy
otherwise:
	if prb exists, use new logic
	else legacy
*/

int NdxDmesgUsage(const char* msg) 
{
	if (msg)
		dprintf("error: %s\n", msg);

	dprintf("dmesg[options]\n"
		"-f read file, only legacy\n"
		"-a address of prb, non - legacy\n"
		"-v verbose mode\n"
		"-l force legacy format\n"
		"otherwise :\n"
		"	if prb exists, use new logic\n"
		"	else legacy\n");
	return -1;
};
/*
int NdxPickNextField(const char* cursor, char* buffer, size_t buf_len, size_t* eaten_length)
{
	int len;
	const char* start = cursor;
	while (*start == ' ') start++;
	const char* sep = strchr(start, ' ');
	len = (sep != NULL) ? (sep - start) : strlen(start);
	if (len < buf_len) {
		strncpy(buffer, start, len);
		buffer[len] = 0;
		*eaten_length = start - cursor + len;
		return len;
	}
	return 0;
}
*/
DECLARE_API(dmesg)
{
	uint64_t prb_addr = 0;
	uint64_t prb_ptr = 0;
	PCSTR re;
	uint64_t Address = 0;
	const char* option=NULL, * filename=NULL;
	char options[256];
	// check arguments
	int num = 0, argc = 1, len = 0;
	const char* cursor = args;
	uint32_t flags = 0;
	size_t eaten_length = 0;

	do {
		switch (cursor[1]) {
		case 'f':
			cursor += 2;
			len = NdxPickNextField(cursor, options, sizeof(options), &eaten_length);
			if (len > 0) {
				filename = strtoull(options, NULL, 0);
			}
			break;
		case 'a':
			cursor += 2;
			len = NdxPickNextField(cursor, options, sizeof(options), &eaten_length);
			if (len > 0) {
				Address = strtoull(options, NULL, 0);
			}
			break;
		case 'v':
			cursor += 2;
			eaten_length = 0;
			dmesg_flags |= NDX_DMESG_OPTION_VERBOSE;
			break;
		case 'l':
			cursor += 2;
			eaten_length = 0;
			dmesg_flags |= NDX_DMESG_OPTION_LEGACY;
			break;
		case 'h':
		case 'H':
		case '?':
			NdxDmesgUsage(NULL);
			return;
		default:
			// old syntax
			len = NdxPickNextField(cursor, options, sizeof(options), &eaten_length);
			if (argc == 1) {
				if (strlen(options) > 7)
					Address = strtoull(options, NULL, 0);
				argc++;
			}
			else if (argc == 2) {
				filename = strtoull(options, NULL, 0);
				argc++;
			}
			else {
				dprintf("too many arguments, ignored\n");
			}
			break;
		}
		cursor += eaten_length;
		while (*cursor == ' ') cursor++;
	}while (*cursor != 0);


	if (filename != NULL || (NDX_DMESG_OPTION_LEGACY& dmesg_flags) != 0) {
		goto legacy;
	}
	if (Address != 0) {
		prb_ptr = Address;
		goto new_logic;
	}
	GetExpressionEx("lk!prb", &prb_addr, &re);
	if (prb_addr == 0) {
	legacy:
		option = strstr(args, "-f");
		if (option != NULL) {
			filename = strchr(option, ' ');
			if (filename == NULL) {
				dprintf("Missing file to decode (%s)\n", args);
				return;
			}
			filename++;
			//GetFieldValue(Address, "kmem_cache_node", "next", Address);
			dmesg_decode_file(filename);
		}
		else {
			dmesg_decode_memory();
		}
	}
	else {
		ReadPointer(prb_addr, &prb_ptr);
		// dprintf("prb_ptr : %p  ", prb_ptr);
	new_logic:
		ndx_dmesgx(prb_ptr);
	}
}
/*
ULONG NdxGetOffset(IN LPCSTR Type, IN LPCSTR Field)
{
	ULONG FieldOffset = 0;
	HRESULT hr = GetFieldOffset(Type, Field, &FieldOffset);
	if (hr != S_OK) {
		dprintf("ndx failed to get %s address for 0x%x", Field, hr);
	}
	return FieldOffset;
}
*/