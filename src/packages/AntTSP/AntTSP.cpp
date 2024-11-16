
#include <efi.h>
#include <efilib.h>

const int max_n = 4000;
const int max_ant = 2000;
double tau[max_n][max_n];
double eta[max_n][max_n];
double len[max_n][max_n];
double rho = 0.5;
double Q = 100;
int n, m, nc;

int rnd = 32767;

int random(int n) {
	rnd = (rnd * 2131 + 7);
	
	return (((rnd * rnd * rnd * rnd) % n) + n) % n;
}

class Ant {
	public:
	int start;
	bool b_can[max_n];
	double t_len = 0;
	
	int path[max_n];
	int path_length;
	
	Ant() {
		for(int i = 0; i < max_n; ++i) b_can[i] = true;
		
		path_length = 0;
	}
	Ant(int _start) {
		start = _start;
		
		path_length = 0;
		path[path_length++] = _start;
		
		for(int i = 0; i < max_n; ++i) b_can[i] = true;
	}
	bool can(int t) {
		return b_can[t];
	}
	
	int next() {
		int s = path[path_length - 1];
		int ci = -1;
		double vmax = 0, vall = 0;
		for(int i = 0; i < n; ++i)
			if(can(i)) {
				vall += tau[s][i] * eta[s][i];
				if((tau[s][i] * eta[s][i]) > vmax || ci < 0) {
					ci = i;
					vmax = tau[s][i] * eta[s][i];
				}
			}
		return ci;
	}
	void reuse() {
		for(int i = 0; i < path_length; ++i) {
			b_can[path[i]] = true;
		}
		
		path_length = 0;
		path[path_length++] = start;
		
		b_can[start] = false;
		t_len = 0;
	}
	double getlen() {
		return t_len;
	}
	void step() {
		double tlen = getlen();
		for(int i = 1; i < path_length; ++i) {
			int src = path[i - 1];
			int dst = path[i];
			tau[src][dst] = tau[src][dst] * rho + Q / tlen;
		}
	}
	void step(int t) {
		path[path_length++] = t;
		b_can[t] = false;
		t_len += len[path[path_length - 2]][path[path_length - 1]];
	}
};

extern "C" int AntTSP_Main() {
	CHAR16 szLine[128];
	
	Input(L"\r\nn >> ", szLine, sizeof(szLine) / sizeof(szLine[0]));
	
	n = Atoi(szLine);

	Input(L"\r\nm >> ", szLine, sizeof(szLine) / sizeof(szLine[0]));
	
	m = Atoi(szLine);

	Input(L"\r\nnc >> ", szLine, sizeof(szLine) / sizeof(szLine[0]));
	
	nc = Atoi(szLine);
	
	for(int i = 0; i < n; ++i)
		for(int j = 0; j < n; ++j) {
			Input(L"\r\np >> ", szLine, sizeof(szLine) / sizeof(szLine[0]));
			
			len[i][j] = Atoi (szLine);
			
			eta[i][j] = 1.0 / len[i][j];
			tau[i][j] = 1.0;
		}

	Ant * ants = new Ant[m];
	for(int i = 0; i < m; ++i) ants[i].start = random(n);
	double sol = 1e+100;
	int NC = 0;
	
	int best_tour[max_n];
	int best_tour_length = 0;
	
	for(NC = 0; NC < nc; NC += best_tour_length == n ? 1 : 0) {
		for(int i = 0; i < m; ++i) ants[i].reuse();
		for(;;) {
			bool b_ok = false;
			int dec[max_ant];
			for(int i = 0; i < m; ++i) {
				dec[i] = ants[i].next();
				if(dec[i] >= 0) {
					b_ok = true;
					ants[i].step(dec[i]);
				}
			}
			if(!b_ok) break;
		}
		for(int i = 0; i < m; ++i) {
			ants[i].step();
			if(ants[i].path_length == n) {
				double tlen = ants[i].getlen();
				if(tlen < sol) {
					sol = tlen;
					
					best_tour_length = ants[i].path_length;
					
					for (int j = 0; j < best_tour_length; ++j) {
						best_tour[j] = ants[i].path[j];
					}
				}
			}
		}
	}
	
	Print(L"\r\n%d\r\n", (int) sol);
	for(int i = 0; i < best_tour_length; ++i) {
		Print(L"%d ", best_tour[i] + 1);
	}
	Print(L"\r\n");
	return 0;
}
