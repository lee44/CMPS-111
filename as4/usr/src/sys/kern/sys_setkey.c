#include <sys/sysproto.h>
#include <sys/proc.h>
#include <fs/cryptofs/cryptouser.h>
//required for printf
#include <sys/types.h>
#include <sys/systm.h>

#ifndef _SYS_SYSPROTO_H_

struct getkey_args {
    uid_t id;
    unsigned long *k0;
    unsigned long *k1;
};

struct setkey_args {
    unsigned long k0;
    unsigned long k1;
    uid_t id;
};
#endif
/* ARGSUSED */

int sys_getkey(struct thread *td, struct getkey_args *args)
{
    for(int i = 0; i < user_key_count; i++)
    {
        if(args->id == user_keys_array[i].id)
         {
             // bcopy(&(user_keys_array[i].user_key),&(args->k0),32);
             // bcopy(&((user_keys_array[i].user_key) >>= 32),&(args->k1),32);
             args->k0 = &(user_keys_array[i].k0);
             args->k1 = &(user_keys_array[i].k1);
         }
         else 
            return -1;
    }
    return 0;
}

int sys_setkey(struct thread *td, struct setkey_args *args)
{
    printf("Key0 is %lu\n",args->k0);
    printf("Key1 is %lu\n",args->k1);
    printf("UID is %i\n",args->id);

    long long va = (long long) args->k0 << 32 | args->k1;

    struct usr c;
    c.id = args->id;
    c.user_key = va;
    c.k0 = args->k0;
    c.k1 = args->k1;

    if(user_key_count <= 16)
    {
        user_keys_array[user_key_count] = c;
        user_key_count += 1; 
    }
    else
        return 1;

    printf("Key(Long Long): %lld\n",user_keys_array[0].user_key);

    printf("k0 stored in array: %lu\n",user_keys_array[0].k0);
    printf("k1 stored in array: %lu\n",user_keys_array[0].k1);

    printf("UID from array: %i\n",user_keys_array[0].id);
    
    return 0;
}