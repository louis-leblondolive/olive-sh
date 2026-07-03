#include "job_table.h"


//  ----- JOB MANAGEMENT -----------------------------------------------

job_t *job_init(pid_t pg_id, char *job_cmd){

    job_t *res = (job_t*)malloc(sizeof(job_t));
    if(!res) return NULL;

    res->job_id = -1;
    res->pg_id = pg_id;
    res->job_cmd = job_cmd;
    res->status = RUNNING;

    return res;
}


void free_job(job_t *job){
    if(!job) return;

    free(job->job_cmd);
    free(job);
}


//  ----- JOB TABLE OPERATIONS -----------------------------------------------

job_table_t *job_table_init(void){

    job_table_t *res = (job_table_t*)malloc(sizeof(job_table_t));
    if(!res) return NULL;

    res->capacity = 0;
    res->tbl = NULL;

    return res;
}


void free_job_table(job_table_t *job_tbl){

    if(!job_tbl) return;
    if(job_tbl->tbl){
        for (size_t i = 0; i < job_tbl->capacity; i++) free_job(job_tbl->tbl[i]);
        free(job_tbl->tbl);
    }
    free(job_tbl);
}


int job_table_add(job_table_t *job_tbl, job_t *job){
    if(!job_tbl || !job) return -1;

    if(!job_tbl->tbl){
        
        job_tbl->tbl = (job_t**)calloc(2, sizeof(job_t*));
        if(!job_tbl->tbl) return -1;
        
        job_tbl->tbl[0] = NULL;
        job_tbl->tbl[1] = job;
        job_tbl->capacity = 2;

        job->job_id = 1;
        return 1;
    }

    else {

        size_t gap = 1;
        while(gap < job_tbl->capacity && job_tbl->tbl[gap] != NULL) gap ++;

        if(gap >= job_tbl->capacity){   // table is full

            job_t **new_tbl = (job_t**)calloc(2 * job_tbl->capacity, sizeof(job_t*));
            if(!new_tbl) return -1;

            for (size_t i = 1; i < job_tbl->capacity; i++) new_tbl[i] = job_tbl->tbl[i];
            new_tbl[job_tbl->capacity] = job;

            free(job_tbl->tbl);
            job_tbl->tbl = new_tbl;

            job->job_id = job_tbl->capacity;

            job_tbl->capacity *= 2;
            return job->job_id;
        } 

        else {  // an empty slot is available
            job_tbl->tbl[gap] = job;
            job->job_id = gap ;

            return gap;
        }
    }

    return -1; // unreachable
}


void job_table_rm(job_table_t *job_tbl, int job_id){
    if(!job_tbl || !job_tbl->tbl ||
         job_id <= 0 || job_id >= (int)job_tbl->capacity) return;

    free_job(job_tbl->tbl[job_id]);
    job_tbl->tbl[job_id] = NULL;

    if(job_tbl->capacity > 2){

        for (size_t i = job_tbl->capacity / 2; i < job_tbl->capacity; i++){
            if(job_tbl->tbl[i] != NULL) return;
        }
        
        job_t **new_tbl = (job_t**)calloc(job_tbl->capacity / 2, sizeof(job_t*));
        if(!new_tbl) return;

        for(size_t i = 0; i < job_tbl->capacity / 2; i++) new_tbl[i] = job_tbl->tbl[i];

        free(job_tbl->tbl);
        job_tbl->tbl = new_tbl;

        job_tbl->capacity /= 2;
    }

    return;
}