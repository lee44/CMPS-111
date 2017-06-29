#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>
#include "rijndael.h"
#include <sys/syscall.h>
#include <sys/stat.h>

static char rcsid[] = "$Id: encrypt.c,v 1.2 2003/04/15 01:05:36 elm Exp elm $";

#define KEYBITS 128

int hexvalue (char c)
{
  if (c >= '0' && c <= '9') {
    return (c - '0');
  } else if (c >= 'a' && c <= 'f') {
    return (10 + c - 'a');
  } else if (c >= 'A' && c <= 'F') {
    return (10 + c - 'A');
  } else {
    fprintf (stderr, "ERROR: key digit %c isn't a hex digit!\n", c);
    exit (-1);
  }
}

unsigned int int_to_int(unsigned int k) {
    return (k == 0 || k == 1 ? k : ((k % 2) + 10 * int_to_int(k / 2)));
}

int main(int argc, char **argv)
{
  unsigned long rk[RKLENGTH(KEYBITS)];	/* round key */
  unsigned char key[KEYLENGTH(KEYBITS)];/* cipher key */
  char	buf[100];
  int i, nbytes, nwritten , ctr;
  int totalbytes;
  unsigned long	k0, k1, *k0_returned, *k1_returned;
  int fileId;			/* fake (in this example) */
  int nrounds;				// # of Rijndael rounds 
  char *password;			/* supplied (ASCII) password */
  int	fd;
  char *filename, *flag, *inputkey;
  unsigned char filedata[16];
  unsigned char ciphertext[16];
  unsigned char ctrvalue[16];
  struct stat fileStat;
  mode_t mode;
  char key0[9];
  char key1[9];
  unsigned int test;
  uid_t uid;

  if (argc < 3)
  {
    fprintf (stderr, "Usage: %s -e/-d key file\n", argv[0]);
    return 1;
  }

  flag = argv[1];
  inputkey = argv[2];
  filename = argv[3];
  if (strlen(inputkey)!=16){
  	fprintf (stderr, "ERROR: key must be 16 characters!\n");
    exit (-1);
  }

//break up key into two parts
  for (int i = 0; i < 8; i++){
  	hexvalue(inputkey[i]);
  	key0[i] = inputkey[i];
	}
  key0[8] = '\0';

  for (int z = 8; z < 16; z++){
  	hexvalue(inputkey[z]);
  	key1[z-8] = inputkey[z];
  }
  key1[8] = '\0';

//load stat
  	if(stat(filename,&fileStat) < 0){
  		//printf(stderr, "Failed to stat file\n");
  		return 1;
  	}
  	mode = fileStat.st_mode & 07777;
        
  
  //syscall(548,strtoul (key0, NULL, 0),strtoul (key1, NULL, 0), fileStat.st_uid);


//encrypt
  if(flag[0] == '-' && flag[1] == 'e')
  {
  	//check sticky bit
    if (!(fileStat.st_mode & S_ISTXT))
    {
    	fileId = fileStat.st_ino;

        bzero (key, sizeof (key));
	    k0 = strtol (key0, NULL, 16);
	    k1 = strtol (key1, NULL, 16);
	    //printf("K0: %i", k0);
	    //printf("K1: %i", k1);
	    bcopy (&k0, &(key[0]), sizeof (k0));
	    bcopy (&k1, &(key[sizeof(k0)]), sizeof (k1));
	    
	    syscall(548, k0, k1, fileStat.st_uid); // system call, not fully implemented

		syscall(549,fileStat.st_uid,&k0_returned,&k1_returned);

	    printf("Key returned from getkey: %lu   %lu\n",(unsigned long)k0_returned,(unsigned long)k1_returned );
	    /* Print the key, just in case */
	    // for (i = 0; i < sizeof (inputkey); i++) {
	    // 	sprintf (buf+2*i, "%02x", inputkey[sizeof(inputkey)-i-1]);
	    // }
	    // fprintf (stderr, "KEY: %s\n", buf);

	    /*
	    * Initialize the Rijndael algorithm.  The round key is initialized by this
	    * call from the values passed in key and KEYBITS.
	    */
	    nrounds = rijndaelSetupEncrypt(rk, key, KEYBITS);

	    /*
	    * Open the file.
	    */
	    fd = open(filename, O_RDWR);
	    if (fd < 0)
	    {
	     fprintf(stderr, "Error opening file %s\n", argv[2]);
	     return 1;
	    }

	  /* fileID goes into bytes 8-11 of the ctrvalue */
	  bcopy (&fileId, &(ctrvalue[8]), sizeof (fileId));

	  /* This loop reads 16 bytes from the file, XORs it with the encrypted
	     CTR value, and then writes it back to the file at the same position.
	     Note that CTR encryption is nice because the same algorithm does
	     encryption and decryption.  In other words, if you run this program
	     twice, it will first encrypt and then decrypt the file.
	  */
	  for (ctr = 0, totalbytes = 0; /* loop forever */; ctr++)
	  {
	    /* Read 16 bytes (128 bits, the blocksize) from the file */
	    nbytes = read (fd, filedata, sizeof (filedata));
	    if (nbytes <= 0) {
	      break;
	    }
	    if (lseek (fd, totalbytes, SEEK_SET) < 0)
	    {
	      perror ("Unable to seek back over buffer");
	      exit (-1);
	    }

	    /* Set up the CTR value to be encrypted */
	    bcopy (&ctr, &(ctrvalue[0]), sizeof (ctr));

	    /* Call the encryption routine to encrypt the CTR value */
	    rijndaelEncrypt(rk, nrounds, ctrvalue, ciphertext);

	    /* XOR the result into the file data */
	    for (i = 0; i < nbytes; i++) {
	      filedata[i] ^= ciphertext[i];
	    }

	    /* Write the result back to the file */
	    nwritten = write(fd, filedata, nbytes);
	    if (nwritten != nbytes)
	    {
	      fprintf (stderr,
		       "%s: error writing the file (expected %d, got %d at ctr %d\n)",
		       argv[0], nbytes, nwritten, ctr);
	      break;
	    }

	    /* Increment the total bytes written */
	    totalbytes += nbytes;
	  }
	   close (fd);

	   //set sticky bit
	   mode = fileStat.st_mode | S_ISTXT;
       chmod (filename, mode);
    }
    else{
    	fprintf (stderr, "ERROR: file already encrypted!\n");
    	exit (-1);
    }
  }
  

  if(flag[0] == '-' && flag[1] == 'd')
  {
  	if(stat(filename,&fileStat) < 0)    
        return 1;

    if (fileStat.st_mode & S_ISTXT)
    {
    	fileId = fileStat.st_ino;
        mode &= ~S_ISTXT;
        chmod (filename, mode);

        bzero (key, sizeof (key));
	    k0 = strtol (key0, NULL, 16);
	    k1 = strtol (key1, NULL, 16);
	    //printf("K0: %i", k0);
	    //printf("K1: %i", k1);
	    bcopy (&k0, &(key[0]), sizeof (k0));
	    bcopy (&k1, &(key[sizeof(k0)]), sizeof (k1));
	    //syscall(548, k0, k1, fileStat.st_uid); // system call, not fully implemented

	    /* Print the key, just in case */
	    // for (i = 0; i < sizeof (inputkey); i++) {
	    // 	sprintf (buf+2*i, "%02x", inputkey[sizeof(inputkey)-i-1]);
	    // }
	    // fprintf (stderr, "KEY: %s\n", buf);

	    /*
	    * Initialize the Rijndael algorithm.  The round key is initialized by this
	    * call from the values passed in key and KEYBITS.
	    */
	    nrounds = rijndaelSetupEncrypt(rk, key, KEYBITS);

	    /*
	    * Open the file.
	    */
	    fd = open(filename, O_RDWR);
	    if (fd < 0)
	    {
	     fprintf(stderr, "Error opening file %s\n", argv[2]);
	     return 1;
	    }

	  /* fileID goes into bytes 8-11 of the ctrvalue */
	  bcopy (&fileId, &(ctrvalue[8]), sizeof (fileId));

	  /* This loop reads 16 bytes from the file, XORs it with the encrypted
	     CTR value, and then writes it back to the file at the same position.
	     Note that CTR encryption is nice because the same algorithm does
	     encryption and decryption.  In other words, if you run this program
	     twice, it will first encrypt and then decrypt the file.
	  */
	  for (ctr = 0, totalbytes = 0; /* loop forever */; ctr++)
	  {
	    /* Read 16 bytes (128 bits, the blocksize) from the file */
	    nbytes = read (fd, filedata, sizeof (filedata));
	    if (nbytes <= 0) {
	      break;
	    }
	    if (lseek (fd, totalbytes, SEEK_SET) < 0)
	    {
	      perror ("Unable to seek back over buffer");
	      exit (-1);
	    }

	    /* Set up the CTR value to be encrypted */
	    bcopy (&ctr, &(ctrvalue[0]), sizeof (ctr));

	    /* Call the encryption routine to encrypt the CTR value */
	    rijndaelEncrypt(rk, nrounds, ctrvalue, ciphertext);

	    /* XOR the result into the file data */
	    for (i = 0; i < nbytes; i++) {
	      filedata[i] ^= ciphertext[i];
	    }

	    /* Write the result back to the file */
	    nwritten = write(fd, filedata, nbytes);
	    if (nwritten != nbytes)
	    {
	      fprintf (stderr,
		       "%s: error writing the file (expected %d, got %d at ctr %d\n)",
		       argv[0], nbytes, nwritten, ctr);
	      break;
	    }

	    /* Increment the total bytes written */
	    totalbytes += nbytes;
	  }
	   close (fd);
    }
    else{
    	fprintf (stderr, "ERROR: file is not encrypted!\n");
    	exit (-1);
    }
  }

}
