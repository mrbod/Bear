/*  Copyright (C) 2012-2017 by László Nagy
    This file is part of Bear.

    Bear is a tool to generate compilation database for clang tooling.

    Bear is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Bear is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[], char *envp[])
{
    int opt;
    char * library = 0;
    char * target = 0;
    char ** command = 0;

    while ((opt = getopt(argc, argv, "l:t:")) != -1) {
        switch (opt) {
            case 'l':
                library = optarg;
                break;
            case 't':
                target = optarg;
                break;
            default: /* '?' */
                fprintf(stderr, "Usage: %s [-t target_url] [-l path_to_libear] command\n",
                        argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Expected argument after options\n");
        exit(EXIT_FAILURE);
    } else {
        command = argv + optind;
    }

    printf("library=%s; target=%s\n", library, target);
    printf("command argument:");
    for (char ** it = command; *it != 0; ++it) {
        printf(" %s", *it);
    }
    printf("\n");

    exit(EXIT_SUCCESS);
}
