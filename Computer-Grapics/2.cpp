#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>
using namespace std;

const string RED = "\033[31m";
const string RESET = "\033[0m";
const int MAX_LINES = 10;
const int MAX_WORDS = 50;

// 문자열을 공백 기준으로 단어 분리 (여러 공백은 하나로 처리)
int split_words(const string& line, string words[], int max_words) {
    istringstream iss(line);
    string word;
    int count = 0;
    while (iss >> word && count < max_words) {
        words[count++] = word;
    }
    return count;
}

string to_upper(const string& s) {
    string res = s;
    transform(res.begin(), res.end(), res.begin(), ::toupper);
    return res;
}

string to_lower(const string& s) {
    string res = s;
    transform(res.begin(), res.end(), res.begin(), ::tolower);
    return res;
}

string highlight_capital_words(const string& line, int& count) {
    string words[MAX_WORDS];
    int n = split_words(line, words, MAX_WORDS);
    stringstream ss;
    for (int i = 0; i < n; ++i) {
        if (!words[i].empty() && isupper(words[i][0])) {
            ss << RED << words[i] << RESET;
            count++;
        }
        else {
            ss << words[i];
        }
        if (i != n - 1) ss << " ";
    }
    return ss.str();
}

string reverse_line(const string& line) {
    string res = line;
    reverse(res.begin(), res.end());
    return res;
}

string insert_slash(const string& line) {
    stringstream ss;
    for (size_t i = 0; i < line.size(); ++i) {
        if (isspace(line[i])) {
            ss << "/";
            while (i + 1 < line.size() && isspace(line[i + 1])) i++;
        }
        else {
            ss << line[i];
        }
    }
    return ss.str();
}

string reverse_words(const string& line, char sep = ' ') {
    string words[MAX_WORDS];
    int n = 0;
    stringstream ss(line);
    string word;
    while (getline(ss, word, sep) && n < MAX_WORDS) {
        words[n++] = word;
    }
    stringstream out;
    for (int i = n - 1; i >= 0; --i) {
        out << words[i];
        if (i != 0) out << sep;
    }
    return out.str();
}

string replace_char(const string& line, char old_c, char new_c) {
    string res = line;
    replace(res.begin(), res.end(), old_c, new_c);
    return res;
}

string move_after_number(const string& line) {
    string res;
    bool found = false;
    for (size_t i = 0; i < line.size(); ++i) {
        res += line[i];
        if (isdigit(line[i]) && !found) {
            res += "\n";
            found = true;
        }
    }
    return res;
}

int count_alpha(const string& line) {
    return count_if(line.begin(), line.end(), ::isalpha);
}

string highlight_word(const string& line, const string& word, int& count) {
    string words[MAX_WORDS];
    int n = split_words(line, words, MAX_WORDS);
    stringstream ss;
    string target = to_lower(word);
    for (int i = 0; i < n; ++i) {
        string w = to_lower(words[i]);
        if (w == target) {
            ss << RED << words[i] << RESET;
            count++;
        }
        else {
            ss << words[i];
        }
        if (i != n - 1) ss << " ";
    }
    return ss.str();
}

// 파일 생성 (예시 데이터)
void create_file(const string& filename) {
    ofstream ofs(filename);
    string data[MAX_LINES] = {
        "Hello world 123",
        "C++ is fun!",
        "Test sentence 456",
        "Visual Studio 2022",
        "Programming is 789 great",
        "Special  *chars* here",
        "Numbers 101 and 202",
        "UpperCase Word",
        "lowercase word",
        "End of file 999"
    };
    for (int i = 0; i < MAX_LINES; ++i) ofs << data[i] << endl;
    ofs.close();
}

int main() {
    //cout << "파일을 생성합니다: data.txt" << endl;
    //create_file("data.txt");

    ifstream ifs("data.txt");
    if (!ifs) {
        cout << "파일을 열 수 없습니다." << endl;
        return 1;
    }

    string lines[MAX_LINES];
    string orig_lines[MAX_LINES];
    string line;
    int line_count = 0;
    while (getline(ifs, line) && line_count < MAX_LINES) {
        lines[line_count] = line;
        orig_lines[line_count] = line;
        line_count++;
    }
    ifs.close();

    // 상태 변수
    bool upper = false, highlight_cap = false, reversed = false, slash = false, reversed_words = false, replaced = false, moved = false;
    bool sorted = false, sort_desc = false;
    char old_c, new_c;
    string search_word;

    while (true) {
        for (int i = 0; i < line_count; ++i) cout << lines[i] << endl;
        cout << "\n명령어를 입력하세요 (a~i, J, q): ";
        char cmd;
        cin >> cmd;

        if (cmd == 'q') break;

        if (cmd == 'a') {
            upper = !upper;
            for (int i = 0; i < line_count; ++i)
                lines[i] = upper ? to_upper(lines[i]) : orig_lines[i];
        }
        else if (cmd == 'b') {
            for (int i = 0; i < line_count; ++i) {
                string words[MAX_WORDS];
                int n = split_words(lines[i], words, MAX_WORDS);
                cout << lines[i] << " [" << n << "]" << endl;
            }
        }
        else if (cmd == 'c') {
            highlight_cap = !highlight_cap;
            for (int i = 0; i < line_count; ++i) {
                int cnt = 0;
                string out = highlight_cap ? highlight_capital_words(lines[i], cnt) : orig_lines[i];
                cout << out;
                if (highlight_cap) cout << " (대문자 시작 단어: " << cnt << ")";
                cout << endl;
            }
        }
        else if (cmd == 'd') {
            reversed = !reversed;
            for (int i = 0; i < line_count; ++i)
                lines[i] = reversed ? reverse_line(lines[i]) : orig_lines[i];
        }
        else if (cmd == 'e') {
            slash = !slash;
            for (int i = 0; i < line_count; ++i)
                lines[i] = slash ? insert_slash(lines[i]) : orig_lines[i];
        }
        else if (cmd == 'f') {
            reversed_words = !reversed_words;
            for (int i = 0; i < line_count; ++i) {
                if (slash)
                    lines[i] = reversed_words ? reverse_words(lines[i], '/') : orig_lines[i];
                else
                    lines[i] = reversed_words ? reverse_words(lines[i], ' ') : orig_lines[i];
            }
        }
        else if (cmd == 'g') {
            replaced = !replaced;
            if (replaced) {
                cout << "바꿀 문자와 새 문자 입력: ";
                cin >> old_c >> new_c;
                for (int i = 0; i < line_count; ++i)
                    lines[i] = replace_char(lines[i], old_c, new_c);
            }
            else {
                for (int i = 0; i < line_count; ++i)
                    lines[i] = orig_lines[i];
            }
        }
        else if (cmd == 'h') {
            moved = !moved;
            for (int i = 0; i < line_count; ++i)
                lines[i] = moved ? move_after_number(lines[i]) : orig_lines[i];
        }
        else if (cmd == 'i') {
            if (!sorted) {
                int alpha_counts[MAX_LINES];
                string temp_lines[MAX_LINES];
                for (int i = 0; i < line_count; ++i) {
                    alpha_counts[i] = count_alpha(lines[i]);
                    temp_lines[i] = lines[i];
                }
                // 오름차순 정렬
                for (int i = 0; i < line_count - 1; ++i) {
                    for (int j = i + 1; j < line_count; ++j) {
                        if (alpha_counts[i] > alpha_counts[j]) {
                            swap(alpha_counts[i], alpha_counts[j]);
                            swap(temp_lines[i], temp_lines[j]);
                        }
                    }
                }
                for (int i = 0; i < line_count; ++i) lines[i] = temp_lines[i];
                sorted = true;
                sort_desc = false;
            }
            else if (!sort_desc) {
                int alpha_counts[MAX_LINES];
                string temp_lines[MAX_LINES];
                for (int i = 0; i < line_count; ++i) {
                    alpha_counts[i] = count_alpha(lines[i]);
                    temp_lines[i] = lines[i];
                }
                // 내림차순 정렬
                for (int i = 0; i < line_count - 1; ++i) {
                    for (int j = i + 1; j < line_count; ++j) {
                        if (alpha_counts[i] < alpha_counts[j]) {
                            swap(alpha_counts[i], alpha_counts[j]);
                            swap(temp_lines[i], temp_lines[j]);
                        }
                    }
                }
                for (int i = 0; i < line_count; ++i) lines[i] = temp_lines[i];
                sort_desc = true;
            }
            else {
                for (int i = 0; i < line_count; ++i) lines[i] = orig_lines[i];
                sorted = false;
                sort_desc = false;
            }
        }
        else if (cmd == 'J') {
            cout << "찾을 단어를 입력하세요: ";
            cin >> search_word;
            int total = 0;
            for (int i = 0; i < line_count; ++i) {
                int cnt = 0;
                cout << highlight_word(lines[i], search_word, cnt) << endl;
                total += cnt;
            }
            cout << "단어 개수: " << total << endl;
        }

        if (cmd != 'b' && cmd != 'c' && cmd != 'J') {
            cout << "\n=== 결과 ===" << endl;
            for (int i = 0; i < line_count; ++i) cout << lines[i] << endl;
        }

        system("pause");
        system("cls");
    }
    cout << "프로그램 종료" << endl;
    return 0;
}