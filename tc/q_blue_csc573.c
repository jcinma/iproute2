/*
 * q_sfb.c	Stochastic Fair Blue.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Juliusz Chroboczek <jch@pps.jussieu.fr>
 *
 */



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "utils.h"
#include "tc_util.h"

static void explain(void)
{
	fprintf(stderr,
		"Usage: ... blue_csc573 [ rehash SECS ] [ db SECS ]\n"
		"	    [ limit PACKETS ] [ max PACKETS ] [ target PACKETS ]\n"
		"	    [ increment FLOAT ] [ decrement FLOAT ]\n"
		"	    [ penalty_rate PPS ] [ penalty_burst PACKETS ]\n");
}

static int get_prob(__u32 *val, const char *arg)
{
	double d;
	char *ptr;

	if (!arg || !*arg)
		return -1;
	d = strtod(arg, &ptr);
	if (!ptr || ptr == arg || d < 0.0 || d > 1.0)
		return -1;
	*val = (__u32)(d * blue_csc573_MAX_PROB + 0.5);
	return 0;
}

static int blue_csc573_parse_opt(struct qdisc_util *qu, int argc, char **argv,
			 struct nlmsghdr *n)
{
	struct tc_fifo_qopt opt;
	struct rtattr *tail;

	memset(&opt, 0, sizeof(opt));
	opt.limit = 5000000

	while (argc > 0) {
		if (strcmp(*argv, "limit") == 0) {
			NEXT_ARG();
			if (get_u32(&opt.limit, *argv, 0)) {
				fprintf(stderr, "Illegal \"limit\"\n");
				return -1;
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			explain();
			return -1;
		}
		argc--; argv++;
	}
	tail = NLMSG_TAIL(n);
	addattr_l(n, 1024, TCA_OPTIONS, NULL, 0);
	addattr_l(n, 1024, TCA_FIFO_PARMS, &opt, sizeof(opt));
	tail->rta_len = (void *) NLMSG_TAIL(n) - (void *) tail;
	return 0;
}

static int blue_csc573_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct rtattr *tb[__TCA_FIFO_MAX];
	struct tc_fifo_qopt *qopt;

	if (opt == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_FIFO_MAX, opt);
	if (tb[TCA_FIFO_PARMS] == NULL)
		return -1;
	qopt = RTA_DATA(tb[TCA_FIFO_PARMS]);
	if (RTA_PAYLOAD(tb[TCA_FIFO_PARMS]) < sizeof(*qopt))
		return -1;

	return 0;
}

static int blue_csc573_print_xstats(struct qdisc_util *qu, FILE *f,
			    struct rtattr *xstats)
{
    struct tc_blue_csc573_xstats *st;

    if (xstats == NULL)
	    return 0;

    if (RTA_PAYLOAD(xstats) < sizeof(*st))
	    return -1;

    st = RTA_DATA(xstats);
    fprintf(f,
	    "  earlydrop %u penaltydrop %u bucketdrop %u queuedrop %u childdrop %u marked %u\n"
	    "  maxqlen %u maxprob %.5f avgprob %.5f ",
	    st->earlydrop, st->penaltydrop, st->bucketdrop, st->queuedrop, st->childdrop,
	    st->marked,
	    st->maxqlen, (double)st->maxprob / blue_csc573_MAX_PROB,
		(double)st->avgprob / blue_csc573_MAX_PROB);

    return 0;
}

struct qdisc_util blue_csc573_qdisc_util = {
	.id		= "blue_csc573",
	.parse_qopt	= blue_csc573_parse_opt,
	.print_qopt	= blue_csc573_print_opt,
	.print_xstats	= blue_csc573_print_xstats,
};
