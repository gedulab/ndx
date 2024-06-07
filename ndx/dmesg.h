#pragma once
#define NDX_DMESG_OPTION_VERBOSE 1
#define NDX_DMESG_OPTION_LEGACY  2

#ifdef __cplusplus
extern "C" {
#endif
#include<stdint.h>
	enum log_flags {
		LOG_NEWLINE = 2,/* text ended with a newline */
		LOG_CONT = 8, /* text is a fragment of a continuation line */
	};

typedef struct printk_log {
	uint64_t ts_nsec; /* timestamp in nanoseconds */
	uint16_t len; /* length of entire record */
	uint16_t text_len; /* length of text buffer */
	uint16_t dict_len; /* length of dictionary buffer */
	uint8_t facility; /* syslog facility */
	uint8_t flags : 5; /* internal record flags */
	uint8_t level : 3; /* syslog level */
#ifdef CONFIG_PRINTK_CALLER
	u32 caller_id;            /* thread id or processor id */
#endif
}printk_log_t;
#ifdef CONFIG_HAVE_EFFICIENT_UNALIGNED_ACCESS
__packed __aligned(4)
#endif

#ifdef __cplusplus
}
#endif