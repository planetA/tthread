#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>

#include "simdag/simdag.h"
#include "numasim.h"

/** Generate jedule file after tasks have run.
 *   Code from http://lists.gforge.inria.fr/pipermail/simgrid-user/2010-June/002042.html
 */
void dump_jedule(char* filename, xbt_dynar_t dax)
{
  unsigned int i, j, k;
  unsigned int current_nworkstations;
  FILE*out = fopen(filename,"w");
  const unsigned nworkstations = SD_workstation_get_number();
  const SD_workstation_t * workstations = SD_workstation_get_list();
  SD_task_t task;
  SD_workstation_t * list;

  fprintf(out,"<?xml version=\"1.0\"?>\n");
  fprintf(out,"<grid_schedule>\n");
  fprintf(out," <grid_info>\n");
  fprintf(out," <info name=\"nb_clusters\" value=\"1\"/>\n");
  fprintf(out," <clusters>\n");
  fprintf(out," <cluster id=\"1\" hosts=\"%d\" first_host=\"0\"/>\n",
          nworkstations);
  fprintf(out," </clusters>\n");
  fprintf(out," </grid_info>\n");
  fprintf(out," <node_infos>\n");

  xbt_dynar_foreach(dax, i, task){
    fprintf(out," <node_statistics>\n");
    fprintf(out," <node_property name=\"id\" value=\"%s\"/>\n",
            SD_task_get_name(task));
    fprintf(out," <node_property name=\"type\" value=\"");
    if (SD_task_get_kind(task) == SD_TASK_COMP_SEQ)
      fprintf(out,"computation\"/>\n");
    if (SD_task_get_kind(task) == SD_TASK_COMM_E2E)
      fprintf(out,"transfer\"/>\n");

    fprintf(out," <node_property name=\"start_time\" value=\"%.6f\"/>\n",
            SD_task_get_start_time(task));
    fprintf(out," <node_property name=\"end_time\" value=\"%.6f\"/>\n",
            SD_task_get_finish_time(task));
    fprintf(out," <configuration>\n");
    fprintf(out," <conf_property name=\"cluster_id\" value=\"0\"/>\n");

    current_nworkstations = SD_task_get_workstation_count(task);

    fprintf(out," <conf_property name=\"host_nb\" value=\"%d\"/>\n",
            current_nworkstations);

    fprintf(out," <host_lists>\n");
    list = SD_task_get_workstation_list(task);
    for (j=0;j<current_nworkstations;j++){
      for (k=0;k<nworkstations;k++){
        if (!strcmp(SD_workstation_get_name(workstations[k]),
                    SD_workstation_get_name(list[j]))){
          fprintf(out," <hosts start=\"%u\" nb=\"1\"/>\n",k);
          break;
        }
      }
    }
    fprintf(out," </host_lists>\n");
    fprintf(out," </configuration>\n");
    fprintf(out," </node_statistics>\n");
  }
  fprintf(out," </node_infos>\n");
  fprintf(out,"</grid_schedule>\n");
  fclose(out);
}
