#include "record.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int record_init(struct record *r){
    r->length = 0;
    r->type = 0;
    r->hasfooter = 0;
    r->payload = NULL;
    r->uuid = 0;
    return 0;
}

void record_free(struct record *r){
    if (r->length > 0){
        free(r->payload);
        r->length = 0;
    }
}

int record_get_type(const struct record *r){
    return r->type;
}

void record_set_type(struct record *r, int type){
    if (type <= 0x7FFF){
        r->type = type;
    }
}

int record_get_length(const struct record *r){
    return r->length;
}

int record_set_payload(struct record *r, const char * buf, int n){
    if (buf == NULL){
        record_free(r);
        return 0;
    }
    r->payload = (char *)malloc(n);
    if (r->payload == NULL){
        return -1;
    }
    memcpy(r->payload, buf, n);
    r->length = n;
    return 0;
}

int record_get_payload(const struct record *r, char *buf, int n){
    if (r->length < n){
        memcpy(buf, r->payload, r->length);
        n = r->length;
    }
        memcpy(buf, r->payload, n);
        return n;
}

int record_has_footer(const struct record *r){
    return r->hasfooter;
}

void record_delete_footer(struct record *r){
    r->hasfooter = 0;
    r->uuid = 0;
}

void record_set_uuid(struct record *r, unsigned int uuid){
        r->hasfooter = 1;
        r->uuid = uuid;
}

unsigned int record_get_uuid(const struct record *r){
    if (r->hasfooter != 1){
        return 0;
    }
    else {
        return r->uuid;
    }
}

int record_write(const struct record *r, FILE *f){
    int count = 0;
    int ret = 0;
    if ((ret = fwrite(&(r->hasfooter), sizeof(unsigned int), 1, f)) <= 0){
        return -1;
    }
    count = count + ret;
    if ((ret = fwrite(&(r->type), sizeof(int), 15, f)) <= 0){
        return -1;
    }
    count = count + ret;
    if ((ret = fwrite(&(r->length), sizeof(uint16_t), 1, f)) <= 0){
        return -1;
    }
    count = count + ret;
    if  (r->length != 0){
        if ((ret = fwrite(r->payload, sizeof(char), r->length, f)) <= 0){
            return -1;
        }
    count = count + ret;
    }
    if (r->hasfooter){
        if ((ret = fwrite(&(r->uuid), sizeof(uint32_t), 32, f)) <= 0){
        	return -1;
    	}
        count = count + ret;
    }
    return count;
}

int record_read(struct record *r, FILE *f){
    if (fread(&(r->hasfooter), sizeof(unsigned int), 1, f) <= 0){
        return -1;
    }
    if (fread(&(r->type), sizeof(int), 15, f) <= 0){
        return -1;
    }
    if (fread(&(r->length), sizeof(unsigned int), 1, f) <= 0){
        return -1;
    }
    if (fread(r->payload, sizeof(char), r->length, f) <= 0){
        return -1;
    }
    if (r->hasfooter){
        if (fread(&(r->uuid), sizeof(uint32_t), 32, f) <= 0){
        	return -1;
    	}
    }
    return r->length;
}
