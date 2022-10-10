#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

const int HISTORY_BIT = 2;

enum Counter { SN, WN, WT, ST };
char table[] = {'N', 'N', 'T', 'T'};
string dash(77, '-');

int main() {
    string input("TTTTNNTNTNTNTNTNT");

    vector<bool> historyBits(2, false);
    vector<Counter> counters(static_cast<int>(pow(2, HISTORY_BIT)), SN);

    cout << '\n'
        << setw(28) << "Counter" << setw(18) << "Predict" << setw(11) << "Real" << setw(29) << "Mispredict or not\n";

    for (size_t i = 0; i < input.size(); ++i) {
        int history = 0;
        history = ((history | historyBits[0]) << 1) | historyBits[1];

        cout << setw(15) << '(';

        cout << historyBits[0] << historyBits[1] << ", ";
        for (int j = 0; j < 4; j++) {
            switch (counters[j]) {
                case 0:
                cout << "SN";
                break;
                case 1:
                cout << "WN";
                break;
                case 2:
                cout << "WT";
                break;
                case 3:
                cout << "ST";
                break;
                default:
                break;
            }
            if (j != 3) cout << ", ";
        }
        cout << ')';

        cout << setw(10) << table[counters[history]];  // predict

        cout << setw(11) << input[i];  // real condition

        if (table[counters[history]] == input[i])  // determine weather it is correct
            cout << setw(31) << "correct\n";
        else
            cout << setw(31) << "mispredict\n";

        cout << setw(8) << " " << dash << '\n';

        historyBits[0] = historyBits[1];
        if (input[i] == 'T') {
            if (counters[history] != 3) counters[history] = static_cast<Counter>(counters[history] + 1);
            historyBits[1] = true;
        } 
        else {
            if (counters[history] != 0) counters[history] = static_cast<Counter>(counters[history] - 1);
            historyBits[1] = false;
        }
    }
}
