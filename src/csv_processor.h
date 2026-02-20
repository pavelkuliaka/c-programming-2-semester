#pragma once

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int isNumber(const char*);

int countCommas(const char*);

int processCSV(const char*, const char*);
