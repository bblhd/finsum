#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

typedef signed long money_t;

typedef _Bool bool;
#define FALSE 0
#define TRUE 1

typedef struct {
	unsigned int amount;
	enum {ONCE, DAYS, MONTHS, YEARS} when;
} timing_t;

typedef struct {
	unsigned int day;
	unsigned int month;
	unsigned int year;
} date_t;

void die(char *format, ...);

money_t positive(money_t x);

bool dateeq(date_t a, date_t b);
bool dategt(date_t a, date_t b);
bool datege(date_t a, date_t b);
date_t datemin(date_t a, date_t b);
date_t dateinc(date_t date);
date_t dateval(date_t date);
date_t datecur();

unsigned int daysInMonth(unsigned int month, unsigned int year);

char *openfile(char *path);
money_t sumEntries(char *in, date_t query);
void requiredWord(char *in, char **out, const char *word);
money_t parseAmount(char *in, char **out);
timing_t parseRepetition(char *in, char **out);
date_t parseDate(char *in, char **out);

int main(int argc, char **argv) {
	if (argc >= 2 && argc <= 5) {
		char *file = openfile(argv[1]);
		date_t query = datecur();
		if (argc >= 3) {
			query.day = strtol(argv[2], NULL, 10);
		}
		if (argc >= 4) {
			if (strcmp(argv[3], "jan")==0) query.month = 1;
			else if (strcmp(argv[3], "feb")==0) query.month = 2;
			else if (strcmp(argv[3], "mar")==0) query.month = 3;
			else if (strcmp(argv[3], "apr")==0) query.month = 4;
			else if (strcmp(argv[3], "may")==0) query.month = 5;
			else if (strcmp(argv[3], "jun")==0) query.month = 6;
			else if (strcmp(argv[3], "jul")==0) query.month = 7;
			else if (strcmp(argv[3], "aug")==0) query.month = 8;
			else if (strcmp(argv[3], "sep")==0) query.month = 9;
			else if (strcmp(argv[3], "oct")==0) query.month = 10;
			else if (strcmp(argv[3], "nov")==0) query.month = 11;
			else if (strcmp(argv[3], "dec")==0) query.month = 12;
			else query.month = 0;
		}
		if (argc >= 5) {
			query.year = strtol(argv[4], NULL, 10);
		}
		money_t amount = sumEntries(file, dateval(query));
		printf("%c$%li.%li\n", amount<0?'-':'+', positive(amount)/100, positive(amount)%100);
	} else {
		printf("Usage:\n");
		printf("\t%s file [day [month [year]]]\n", argv[0]);
		printf("Description:\n");
		printf("\tThis program calculates a sum of transactions which have been stored in a file.\n");
		printf("\tIt is intended to be used for simple single account budgeting in the modern era.\n");
		printf("File format:\n");
		printf("\tUses a human readable file format, consisting of a series of entries seperated by line breaks.\n");
		printf("\tEach entry is formatted according to this EBNF desciption:\n\n");
		printf("\t\tentry ::= amount when ( comment )?\n");
		printf("\t\tamount ::= ( '+' | '-' ) '$' decimal \n");
		printf("\t\twhen ::= 'every' timing 'from' date ( 'to' date )? | 'on' date \n");
		printf("\t\tcomment ::= '#' .* \n");
		printf("\t\ttiming ::= 'day' | 'month' | 'year' | integer ('days' | 'months' | 'years')\n");
		printf("\t\tdate ::= integer month integer\n");
		printf("\t\tdecimal ::= [0-9]* ( '.' ( [0-9] [0-9]? )? )?\n");
		printf("\t\tinteger ::= [0-9]*\n");
		printf("\t\tmonth ::= jan | feb | mar | apr | may | jun | jul | aug | sep | oct | nov | dec\n");\
		printf("Entry Examples:\n");
		printf("\t-$50.30 every 2 weeks from 9 aug 2020\n");
		printf("\t+$50000 every year from 1 jan 2014 to 1 jan 2020\n");
		printf("\t-$12 on 17 sep 2019 #lunch\n");
		printf("Usage Examples:\n");
		printf("\t%s finances.txt               calculates sum of transactions up to the current date\n", argv[0]);
		printf("\t%s finances.txt 9 jun         calculates sum of transactions up to the 9th of june, the same year\n", argv[0]);
		printf("\t%s finances.txt 23 mar 2022   calculates sum of transactions up to the 23rd of march, 2022\n", argv[0]);
	}
}

money_t positive(money_t x) {
	if (x < 0) return -x;
	else return x;
}

money_t parseEntry(char *in, char **out, date_t query);

bool stepThroughWhitespace(char **s) {
	bool true = **s == ' ' || **s == '\t';
	while (**s == ' ' || **s == '\t') (*s)++;
	return true;
}

bool stepThroughLineEnd(char **s) {
	bool true = FALSE;
	true = true || stepThroughWhitespace(s);
	true = true || **s == '#';
	if (**s == '#') {
		while (**s != '\n' && **s != '\r' && **s != '\0') (*s)++;
	}
	true = true || **s == '\n' || **s == '\r';
	while (**s == '\n' || **s == '\r') (*s)++;
	return true;
}
bool stepThroughLineEnds(char **s) {
	bool true = FALSE;
	bool v = FALSE;
	while ((v = stepThroughLineEnd(s))) true = true || v;
	return true;
}

money_t sumEntries(char *in, date_t query) {
	money_t amount = 0;
	while (*in != '\0') {
		amount += parseEntry(in, &in, query);
		stepThroughLineEnds(&in);
	}
	return amount;
}

money_t parseEntry(char *in, char **out, date_t query) {
	money_t amount = parseAmount(in, &in);
	timing_t timing = parseRepetition(in, &in);
	
	if (timing.when == ONCE) {
		requiredWord(in, &in, "on");
		if (dategt(parseDate(in, &in), query)) amount = 0;
	} else {
		requiredWord(in, &in, "from");
		date_t start = parseDate(in, &in);
		requiredWord(in, &in, "to");
		date_t end = datemin(parseDate(in, &in), query);
		int count = 0;
		unsigned int ticker = timing.amount;
		for (date_t d = dateinc(start); datege(end, d); d = dateinc(d)) {
			if (ticker >= timing.amount) {
				ticker = 0;
				count++;
			}
			if (timing.when == DAYS) {
				ticker++;
			} else if (timing.when == MONTHS) {
				if (d.day == start.day) ticker++;
			} else if (timing.when == YEARS) {
				if (d.day == start.day && d.month == start.month) ticker++;
			}
		}
		amount *= count;
	}
	if (out) *out = in;
	return amount;
}

bool prefix(char *in, char **out, const char *pre) {
	size_t len = strlen(pre);
	bool present = strncmp(pre, in, len) == 0;
	if (present) in += len;
	if (out) *out = in;
    return present;
}

long parseFixedPrecisionNumber(char *in, char **out, long precision) {
	long amount = 0;
	
	while (*in >= '0' && *in <= '9') {
		amount *= 10;
		amount += (*in - '0') * precision;
		in++;
	}
	
	if (*in == '.') {
		in++;
		while (*in >= '0' && *in <= '9' && precision >= 10) {
			precision /= 10;
			amount += (*in - '0') * precision;
			in++;
		}
	}
	
	if (*in >= '0' && *in <= '9') die("Too many digits!");
	
	if (out) *out = in;
	return amount;
}

void requiredWord(char *in, char **out, const char *word) {
	stepThroughWhitespace(&in);
	if (!prefix(in, &in, word)) die("Missing '%s'!", word);
	if (out) *out = in;
}

date_t parseDate(char *in, char **out) {
	date_t date;
	
	stepThroughWhitespace(&in);
	date.day = parseFixedPrecisionNumber(in, &in, 1);
	
	stepThroughWhitespace(&in);
	if (prefix(in, &in, "jan")) date.month = 1;
	else if (prefix(in, &in, "feb")) date.month = 2;
	else if (prefix(in, &in, "mar")) date.month = 3;
	else if (prefix(in, &in, "apr")) date.month = 4;
	else if (prefix(in, &in, "may")) date.month = 5;
	else if (prefix(in, &in, "jun")) date.month = 6;
	else if (prefix(in, &in, "jul")) date.month = 7;
	else if (prefix(in, &in, "aug")) date.month = 8;
	else if (prefix(in, &in, "sep")) date.month = 9;
	else if (prefix(in, &in, "oct")) date.month = 10;
	else if (prefix(in, &in, "nov")) date.month = 11;
	else if (prefix(in, &in, "dec")) date.month = 12;
	else date.month = 0;
	
	stepThroughWhitespace(&in);
	date.year = parseFixedPrecisionNumber(in, &in, 1);
	
	dateval(date);
	
	if (out) *out = in;
	return date;
}

money_t parseAmount(char *in, char **out) {
	stepThroughWhitespace(&in);
	
	money_t sign = 1;
	if (*in == '-') {sign = -1;}
	else if (*in != '+') die("Sign missing!");
	in++;
	
	money_t units = 1;
	if (*in == '$') units = 100;
	else if ((*in >= '0' && *in <= '9') || *in == '.') die("No unit given!");
	else die("Invalid unit (%c)!", *in);
	in++;
	
	money_t value = parseFixedPrecisionNumber(in, &in, units);
	
	if (out) *out = in;
	return value * sign;
}

timing_t parseRepetition(char *in, char **out) {
	stepThroughWhitespace(&in);
	timing_t timing = {1, ONCE};
	if (prefix(in, &in, "every")) {
		stepThroughWhitespace(&in);
		if (prefix(in, &in, "day")) timing.when = DAYS;
		else if (prefix(in, &in, "month")) timing.when = MONTHS;
		else if (prefix(in, &in, "year")) timing.when = YEARS;
		else {
			timing.amount = parseFixedPrecisionNumber(in, &in, 1);
			if (timing.amount == 0) die("Invalid repetition timing!");
			
			stepThroughWhitespace(&in);
			if (prefix(in, &in, "days")) timing.when = DAYS;
			else if (prefix(in, &in, "months")) timing.when = MONTHS;
			else if (prefix(in, &in, "years")) timing.when = YEARS;
			else die("Invalid repetition unit!");
		}
	}
	if (out) *out = in;
	return timing;
}

char *openfile(char *path) {
	char *buffer = NULL;
	FILE *file = fopen(path, "rb");
	
	if (file) {
		fseek(file, 0, SEEK_END);
		size_t size = ftell(file);
		rewind(file);
		if (size > 0) {
			buffer = malloc(size+1);
			if (buffer) {
				fread(buffer, size, 1, file);
				buffer[size] = 0;
			}
		}
		fclose(file);
	}
	return buffer;
}

bool dateeq(date_t a, date_t b) {
	return a.day == b.day && a.month == b.month && a.year == b.year;
}

bool dategt(date_t a, date_t b) {
	return a.year > b.year || (
		a.year == b.year && (a.month > b.month || (
			a.month == b.month && a.day > b.day
		))
	);
}

bool datege(date_t a, date_t b) {
	return !dategt(b, a);
}

date_t datemin(date_t a, date_t b) {
	if (dategt(b, a)) return a;
	else return b;
}

unsigned int daysInMonth(unsigned int month, unsigned int year) {
	if (month < 1 || month > 12) return 0;
	else if (month == 2 && year%4==0 && (year%100!=0 || year%400==0)) return 29;
	else if (month == 2) return 28;
	else if (month == 4 || month == 6 || month == 9 || month == 11) return 30;
	else return 31;
}

date_t dateinc(date_t date) {
	date_t new = date;
	new.day++;
	while (new.day > daysInMonth(new.month, new.year)) {
		new.day -= daysInMonth(new.month, new.year);
		new.month++;
		while (date.month > 12) {
			new.month -= 12;
			new.year++;
		}
	}
	return new;
}

date_t dateval(date_t date) {
	if (date.year < 1) die("Year (%u) too early!", date.year);
	if (date.month < 1 || date.month > 12) die("Month (%u) outside range!", date.month);
	if (date.day < 1) date.day = 1;
	if (date.day > daysInMonth(date.month, date.year)) date.day = daysInMonth(date.month, date.year);
	return date;
}

date_t datecur() {
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	return dateval((date_t) {tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900});
}

void die(char *format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
	va_end(args);
	exit(EXIT_FAILURE);
}
