#ifndef	FS_CRYPTOUSER_H
#define	FS_CRYPTOUSER_H

struct usr{
	uid_t id;
	long long user_key;
	unsigned long k0;
    unsigned long k1;
};
extern struct usr user_keys_array[16];
extern int user_key_count;

#endif