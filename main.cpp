#include "sha1-1.h"
#include <omp.h>
#include <string.h>
#include <fstream>
using namespace std;

void sequential() {
	double tStart = omp_get_wtime();
	string pwdHash = "00c3cc7c9d684decd98721deaa0729d73faa9d9b";
	ifstream f("10-million-password-list-top-1000000.txt", ios::binary);
	string line;
	SHA1 sha;
	int idx = 0;
	while (getline(f, line, '\x0a')) {
		sha.update("parallel" + line);
		string tempRes = sha.final();
		sha.update(tempRes);
		string res = sha.final();
		if (res == pwdHash) {
			double tStop = omp_get_wtime();
			cout << "Found pwd in " << tStop - tStart << " seconds" << endl << "Pwd is " << line << ", found at index " << idx << endl;
			break;
		}
		idx++;
	}
	f.close();
}

void parallel() {
	double tStart = omp_get_wtime();
	string pwdHash = "00c3cc7c9d684decd98721deaa0729d73faa9d9b";
	ifstream f("10-million-password-list-top-1000000.txt", ios::binary);
	string line;
	SHA1 sha;
	int idx = 0;
	bool found = false;
	omp_lock_t mutex;
	omp_init_lock(&mutex);
	omp_set_dynamic(0);
	omp_set_num_threads(omp_get_max_threads());
#pragma omp parallel for private(line, sha) shared (found)
	for (int i = 0; i < 1000000; i++) {
		if (!found) {
			int currIdx = idx + omp_get_num_threads() - 1;
			omp_set_lock(&mutex);
			getline(f, line, '\x0a');
			omp_unset_lock(&mutex);
			sha.update("parallel" + line);
			string tempRes = sha.final();
			sha.update(tempRes);
			string res = sha.final();
			if (res == pwdHash) {
				double tStop = omp_get_wtime();
				cout << "Found pwd in " << tStop - tStart << " seconds" << endl << "Pwd is " << line << ", found at index " << currIdx << endl;
				found = true;
				break;
			}
			#pragma omp atomic
			idx += 1;
		}
	}
	f.close();

}

int main(int argc, char** argv) {
	cout << "------- Sequential run -------" << endl;
	sequential();
	cout << "------- Parallel run -------" << endl;
	parallel();
	return 0;
}