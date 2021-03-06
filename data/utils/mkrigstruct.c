/*

   mkrigstruct.c - Copyright (C) 2005 Joop Stakenborg <pg4i@amsat.org>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Xlog is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with xlog.  If not, see <http://www.gnu.org/licenses/>.
  
 */

/* compile with: "gcc -o mkrigstruct mkrigstruct.c -lhamlib" */

#include <stdio.h>
#include <hamlib/rig.h>

int print_caps_sum (const struct rig_caps *caps, void *data)
{

	printf ("\t{%d, \"%s\"},\n",caps->rig_model,caps->model_name);
	return -1;	/* !=0, we want them all ! */
}


int main (int argc, char *argv[])
{ 
	int status;

	rig_set_debug (RIG_DEBUG_NONE);
	rig_load_all_backends ();
	printf ("/* The following 2 structs are generated by mkrigstruct */\n"
			"struct rig_id {\n"
			"\tconst gint modelnr;\n"
			"\tconst gchar *modelname;\n"
			"};\n\n");
	printf ("static const struct rig_id rig_id_list[] = {\n");
	status = rig_list_foreach (print_caps_sum, NULL);
	printf ("\t{0, NULL}, /* end marker */\n"
			"};\n");
	return 0;
}


