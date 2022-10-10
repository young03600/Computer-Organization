// AlCO_project3.cpp : 此檔案包含 'main' 函式。程式會於該處開始執行及結束執行。
//

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <queue>
using namespace std;

struct AddType {
	char oprator;

	string rd;
	string operand1;
	string operand2;

	bool busy;
	bool dispatch;
	int dispatch_cycle;

	AddType() : oprator(' '), rd(""), operand1(""), operand2(""), busy(false), dispatch(false), dispatch_cycle(-1) {}
};

struct MulType {
	char oprator;
	
	string rd;
	string operand1;
	string operand2;

	bool busy;
	bool dispatch;
	int dispatch_cycle;

	MulType() : oprator(' '), rd(""), operand1(""), operand2(""), busy(false), dispatch(false), dispatch_cycle(-1) {}
};

struct Inst {

	Inst(string &_op, string &_rd, string &_rs1, string &_rs2) : op(move(_op)), rd(move(_rd)), rs1(move(_rs1)), rs2(move(_rs2)) {}
	
	Inst(const Inst &_inst) {
		op = _inst.op;
		rd = _inst.rd;
		rs1 = _inst.rs1;
		rs2 = _inst.rs2;
	}

	Inst() {};

	string op;
	string rd;
	string rs1;
	string rs2;
};

bool issue();
void execute();
bool write_back();

void print_rs();
void print_rat();
void print_rf();


vector<AddType> add_rs(3, AddType());
vector<MulType> mul_rs(2, MulType());

vector<int> RF = { 0, 2, 4, 6, 8 };
vector<string> RAT(5, "empty");

queue<Inst> init_queue;

const int add_sub_cycle = 2;
const int mul_cycle = 10;
const int div_cycle = 40;

int num_instruction;
int num_write_back = 0;
int cycle = 0;
int execute_add_rs = -1;
int execute_mul_rs = -1;

bool add_busy = false;
bool mul_busy = false;

int main()
{
	ifstream ifs("input.txt");
	if (!ifs) {
		cerr << "File not found\n";
		return -1;
	}

	string line;
	while (getline(ifs, line)) {
		string op, rd, rs1, rs2;
		istringstream iss(line);
		while (iss >> op >> rd >> rs1 >> rs2) {
			rd.erase(rd.end() - 1);
			rs1.erase(rs1.end() - 1);
			init_queue.push(Inst(op, rd, rs1, rs2));
		}
	}

	num_instruction = init_queue.size();

	bool done = false;
	do {
		
		++cycle;

		bool modified = false;

		if ( issue() ) {
			modified = true;
		}
		
		execute();
		
		if ( write_back() ) {
			modified = true;
		}

		if (modified or ( execute_add_rs and execute_mul_rs ) != -1 ) {
			cout << "Cycle: " << cycle << ":\n\n";

			print_rs();
			print_rat();
			print_rf();
		}


		execute_add_rs = -1;
		execute_mul_rs = -1;

	} while (num_write_back != num_instruction);
}


bool issue() {

	if (!init_queue.empty()) {

		if (init_queue.front().op == "ADD" or init_queue.front().op == "SUB") {
			int count = 1;
			for (auto &rs : add_rs) {
				if (!rs.busy) {

					Inst inst(init_queue.front());
					init_queue.pop();

					rs.busy = true;

					rs.rd = inst.rd;

					if (inst.op == "ADD")
						rs.oprator = '+';
					else
						rs.oprator = '-';

					if (RAT[(inst.rs1[1] - '0') - 1] == "empty")
						rs.operand1 = to_string(RF[(inst.rs1[1] - '0') - 1]);
					else
						rs.operand1 = RAT[(inst.rs1[1] - '0') - 1];

					if (RAT[(inst.rs2[1] - '0') - 1] == "empty")
						rs.operand2 = to_string(RF[(inst.rs2[1] - '0') - 1]);
					else
						rs.operand2 = RAT[(inst.rs2[1] - '0') - 1];

					RAT[(inst.rd[1] - '0') - 1] = "RS" + to_string(count);

					return true;
				}
				else ++count;
			}

		}

		if (init_queue.front().op == "ADDI") {
			int count = 1;
			for (auto &rs : add_rs) {
				if (!rs.busy) {

					Inst inst(init_queue.front());
					init_queue.pop();

					rs.busy = true;

					rs.oprator = '+';

					rs.rd = inst.rd;

					if (RAT[(inst.rs1[1] - '0') - 1] == "empty")
						rs.operand1 = to_string(RF[(inst.rs1[1] - '0') - 1]);
					else
						rs.operand1 = RAT[(inst.rs1[1] - '0') - 1];

					rs.operand2 = inst.rs2;   // addi type  -  const int

					RAT[(inst.rd[1] - '0') - 1] = "RS" + to_string(count);

					return true;
				}
				else ++count;
			}
		}


		if (init_queue.front().op == "MUL" or init_queue.front().op == "DIV") {
			int count = 1;
			for (auto &rs : mul_rs) {
				if (!rs.busy) {

					Inst inst(init_queue.front());
					init_queue.pop();

					rs.busy = true;

					rs.rd = inst.rd;

					if (inst.op == "MUL")
						rs.oprator = '*';
					else
						rs.oprator = '/';

					if (RAT[(inst.rs1[1] - '0') - 1] == "empty")
						rs.operand1 = to_string(RF[(inst.rs1[1] - '0') - 1]);
					else
						rs.operand1 = RAT[(inst.rs1[1] - '0') - 1];

					if (RAT[(inst.rs2[1] - '0') - 1] == "empty")
						rs.operand2 = to_string(RF[(inst.rs2[1] - '0') - 1]);
					else
						rs.operand2 = RAT[(inst.rs2[1] - '0') - 1];

					RAT[(inst.rd[1] - '0') - 1] = "RS" + to_string(3 + count);

					return true;
				}
				else 
					++count;
			}
		}
	}

	return false;
}

void execute() {

	// choose which instruciton in the RS to be dispatched
	int count_rs = 1;
	for (auto &rs : add_rs) {
		if (!add_busy and rs.dispatch_cycle == cycle) {
			add_busy = true;
			rs.dispatch = true;

			execute_add_rs = count_rs;

			break;
		}

		if (!add_busy  \
			and !rs.dispatch \
			and rs.busy \
			and rs.operand1.find("RS") == string::npos and rs.operand2.find("RS") == string::npos ) {
			
			rs.dispatch_cycle = cycle + 1;
		}

		++count_rs;
	}

	count_rs = 4;
	for (auto &rs : mul_rs) {
		if (!mul_busy and rs.dispatch_cycle == cycle) {
			mul_busy = true;
			rs.dispatch = true;

			execute_mul_rs = count_rs;

			break;
		}
		
		if (!mul_busy \
			and !rs.dispatch \
			and rs.busy \
			and rs.operand1.find("RS") == string::npos and rs.operand2.find("RS") == string::npos ) {
				
			rs.dispatch_cycle = cycle + 1;
		}

		++count_rs;
	}

	//return executes;
}

template<typename rs_type>
void flash_rs(rs_type &rs) {
	rs.oprator = ' ';
	rs.rd = "";
	rs.operand1 = "";
	rs.operand2 = "";

	rs.busy = false;
	rs.dispatch = false;
	rs.dispatch_cycle = -1;
}


bool write_back() {

	bool write_backs = false;

	auto broadcast_value = [&](string &&_rs, int &_val) -> bool {
		
		bool broadcast = false;
		// update rs
		for (auto &rs : add_rs) {
			if (rs.operand1.find(_rs) != string::npos) {
				rs.operand1 = to_string(_val);

				broadcast = true;
			}
			if (rs.operand2.find(_rs) != string::npos) {
				rs.operand2 = to_string(_val);

				broadcast = true;
			}
		}

		for (auto &rs : mul_rs) {
			if (rs.operand1.find(_rs) != string::npos) {
				rs.operand1 = to_string(_val);

				broadcast = true;
			}
			if (rs.operand2.find(_rs) != string::npos) {
				rs.operand2 = to_string(_val);

				broadcast = true;
			}
		}
		//

		// update RAT
		for (int i = 0; i < RAT.size(); ++i) {
			if (RAT[i] == _rs) {
				RAT[i] = "empty";
				RF[i] = _val;

				broadcast = true;
			}
		}

		return broadcast;
	};

	
	int val;
	for (int i = 0; i < add_rs.size(); ++i) { // 1, 2, 3
		if (add_rs[i].dispatch 
			and (add_rs[i].dispatch_cycle + add_sub_cycle <= cycle) ) {
			
			if (add_rs[i].oprator == '+') val = stoi(add_rs[i].operand1) + stoi(add_rs[i].operand2);
			else val = stoi(add_rs[i].operand1) - stoi(add_rs[i].operand2);

			if(broadcast_value("RS" + to_string(i + 1), val)) write_backs = true;

			flash_rs(add_rs[i]);

			++num_write_back;
			add_busy = false;

			break;
		}


	}

	for (int i = 0; i < mul_rs.size(); ++i) {   // 4, 5

		if (mul_rs[i].dispatch) {
			if (mul_rs[i].oprator == '*') {
				if ((mul_rs[i].dispatch_cycle + mul_cycle) <= cycle) {
					val = stoi(mul_rs[i].operand1) * stoi(mul_rs[i].operand2);
					if (broadcast_value("RS" + to_string(4 + i), val)) write_backs = true;
					flash_rs(mul_rs[i]);

					++num_write_back;
					mul_busy = false;
				}
			}
			else {
				if ((mul_rs[i].dispatch_cycle + div_cycle) <= cycle) {
					val = stoi(mul_rs[i].operand1) / stoi(mul_rs[i].operand2);
					if (broadcast_value("RS" + to_string(4 + i), val)) write_backs = true;

					flash_rs(mul_rs[i]);

					++num_write_back;
					mul_busy = false;
				}
			}

			break;
		}
	}

	return write_backs;
}

string dash(30, '-');

void print_rs() {

	cout << "Reservation Station:\n";
	cout << dash << '\n';

	int count = 1;
	for (auto &rs : add_rs) {
		if(execute_add_rs == count)
			cout << "RS" << count++ << ": " << setw(2) << rs.busy << ' ' << setw(2) << rs.oprator << ' ' << setw(2) << rs.operand1 << ' ' << setw(2) << rs.operand2 << setw(16) << "V\n";
		else
			cout << "RS" << count++ << ": " << setw(2) << rs.busy << ' ' << setw(2) << rs.oprator << ' ' << setw(2) << rs.operand1 << ' ' << setw(2) << rs.operand2 << '\n';
		cout << dash << '\n';
	}

	for (auto &rs : mul_rs) {
		if (execute_mul_rs == count)
			cout << "RS" << count++ << ": " << setw(2) << rs.busy << ' ' << setw(2) << rs.oprator << ' ' << setw(2) << rs.operand1 << ' ' << setw(2) << rs.operand2 << setw(16) << "V\n";
		else
			cout << "RS" << count++ << ": " << setw(2) << rs.busy << ' ' << setw(2) << rs.oprator << ' ' << setw(2) << rs.operand1 << ' ' << setw(2) << rs.operand2 << '\n';
		cout << dash << '\n';
	}

	cout << '\n';
}

void print_rat() {
	
	cout << "Register Allocation Table:\n";
	cout << dash << '\n';

	int count = 1;
	for (auto &rat : RAT) {
		cout << "F" << count++ << ": " << rat << '\n';

		cout << dash << '\n';
	}
	cout << '\n';
}

void print_rf() {

	cout << "Register File:\n";
	cout << dash << '\n';

	int count = 1;
	for (auto &rf : RF) {
		cout << "F" << count++ << ": " << rf << '\n';

		cout << dash << '\n';
	}

	cout << '\n';
}