#include <iostream>
#include <fstream>
#include <unordered_set>
#include <map>
#include <vector>
#include <set>
#include <queue>
#include <algorithm>
using namespace std;
const int N=1100;  //状态数
const int TerminalFlag=233; //扫描过程结束的标志码
void init(const string& input_path);void scan();void  GetNFA();void NFA_TO_DFA();
void handlerError(const string& ErrorText);bool match_DFA (string sentence="") ;void MovePoint();
void show_dfa_set();void BackPoint();void ShowTokens();void WriteTokens();
ifstream file;
int line_number;
int start_point;
int cur_point;
char ch;
class Token {
        string value;
        int type;
        int line;
        int start_pos;
        int len;
    public:
        Token() {}
        Token(const int type, const string& value) {
            this->value = value;
            this->type = type;
            this->line = line_number;
            this->start_pos = start_point;
            this->len = cur_point-start_point;
        }

            [[nodiscard]] string value1() const {
                return value;
            }

            [[nodiscard]] int type1() const {
                return type;
            }

            [[nodiscard]] int line1() const {
                return line;
            }

            [[nodiscard]] int start_pos1() const {
                return start_pos;
            }

            [[nodiscard]] int len1() const {
                return len;
            }
};

unordered_set<string>KeyWord={"auto", "double", "int", "struct", "break", "else", "long", "switch",
"case", "enum", "register", "typedef", "char", "extern", "return", "union",
"const", "float", "short", "unsigned", "continue", "for", "signed", "void",
"default", "goto", "sizeof", "volatile", "do", "if", "while", "static"
};
unordered_set<char>CalWord={
    '=', '+', '-', '*', '/', '<', '>', '!','|','&','%'
};
unordered_set<char>Delimiter={
    ';','{','}',',','(',')',':','[',']','\'','"'
};//d
unordered_set<char>FilterWord={
    ' ' ,'\t'
};//c
// f 代表 '\n'
// k 代表特殊字符
map<char,int>dfa[N];
map<char,map<char,set<char>>>nfa;
int TerminalState[N];
char nfa_S,ed;
enum {
    token_keyword=1,
    token_identifier,
    token_number,
    token_operator,
    token_delimiter,
    error_number,
    error_identifier,
    error_special,
    comment,
    pre_compilation,
};
vector<Token> tokens;
set<char>FinalSet,StateSet;
vector<set<char>>dfa_set;//nfa->dfa 后的集合
string Word;
int main() {
    const string input_path="test.c";
    init(input_path);
    GetNFA();
    // show_dfa_set();
    scan();
    ShowTokens();
    WriteTokens();
    return 0;
}
void ShowTokens() {
    for (const auto& v:tokens) {
        cout<<v.type1()<<", "<<v.value1()<<endl;
    }
}
void WriteTokens() {
    file.close();
    ofstream output;
    const string OutputPath="result.txt";
    output.open(OutputPath);
    for (const auto& v:tokens) {
        output<<v.type1()<<", "<<v.value1()<<endl;
    }
    cout<<"finish writing in: "<<OutputPath<<endl;
    output.close();
}

void init(const string& input_path) {
    file.open(input_path);
    if (!file) {
        cout<<"not found file!"<<endl;
    }
    else {
        line_number=1;
        cur_point=start_point=0;
        tokens.clear();
        for (int &i:TerminalState) {i=-1;}
        TerminalState[TerminalFlag]=TerminalFlag;
        dfa[0]['@']=TerminalFlag;  //  S->@T
    }
}
void  GetNFA() {
    fstream input("WenFa.txt",std::ios::in);
    int n;
    input>>std::skipws>>n;

    while (n--) {
        bool IsS=true;
        int cnt;
        input>>std::skipws>>cnt>>ed;
        // cout<<cnt<<" "<<ed<<endl;
        while (cnt--) {
            char left;
            string right="";
            char tmp1,tmp2,r0,r1;
            input >> std::skipws>>left>>tmp1>>tmp2;
            input>>std::noskipws>>r0>>r1;
            right+=r0;
            if (r1!='\n')right+=r1;
            if (IsS) {
                nfa_S=left;
                IsS=false;
            }
            StateSet.insert(left);
            FinalSet.insert(right[0]);
            if (right.size()>1) {
                nfa[left][right[0]].insert(right[1]);
            }
            else{
                nfa[left][right[0]].insert(ed);
            }
        }
        NFA_TO_DFA();
    }

}
void get_closure(set<char> &temp) {
    queue<char>q;
    for (auto v:temp)q.push(v);
    while (!q.empty()) {

        char now=q.front();
        q.pop();
        if (nfa[now].count('@')==0)continue;
        for (auto v:nfa[now]['@']) {
            if (temp.count(v)==0)q.push(v);
            temp.insert(v);
        }
    }
}
int find_pos(set<char> After) {
    auto it=find(dfa_set.begin(),dfa_set.end(),After);
    if (it==dfa_set.end())return -1;
    return distance(dfa_set.begin(),it);
}
void show_dfa_set() {
    for (int i=0;i<dfa_set.size();i++) {
        cout<<i<<" set:"<<endl;
        for (auto j:dfa_set[i]) {
            cout<<j<<" ";
        }
        cout<<endl;
    }
}
void PrintSet(const set<char>& temp) {
    cout<<"set:"<<endl;
    for (auto v:temp) {cout<<v<<" ";}
    cout<<endl;
}
int Get_Terminal_type(const char ed) {
    switch (ed) {
        case 'X':
            return comment;
        case 'Y':
            return token_number;
        case 'I':
            return token_identifier;
        case 'T':
            return TerminalFlag;
        case 'O':
            return token_operator;
        case 'Z':
            return token_delimiter;
        case 'K':
            return error_special;
        case 'P':
            return pre_compilation;
        default:
            return 404;
    }
}
void NFA_TO_DFA() {
    set<char>Pre;
    set<char>After;
    Pre.insert(nfa_S);
    get_closure(Pre);
    queue<set<char>>q;
    q.push(Pre);
    if (Pre.count(ed))TerminalState[0]=Get_Terminal_type(ed); //todo：这里有一个弊端，就是所有的状态图起点必须唯一！有问题！这里起点只能去到一个终点！
    if (find_pos(Pre)==-1)dfa_set.emplace_back(Pre);
    // for (auto i:FinalSet) {
    //     cout<<i<<' ';
    // }
    // cout<<endl;
    while (!q.empty()) {
        auto v=q.front();
        q.pop();
        for (auto i:FinalSet) {
            if (i=='@')continue;
            for (auto j:v) {
                if (nfa[j].count(i)==0)continue;
                for (auto k:nfa[j][i]) {
                    After.insert(k);
                }
            }
            // PrintSet(After);
            get_closure(After);
            if (After.empty()) {continue;}
            auto it=find(dfa_set.begin(),dfa_set.end(),After);
            if (it==dfa_set.end()) {
                q.push(After);
                dfa_set.emplace_back(After);
            }
            //找到它的位置！！
            int ids2=find_pos(After);
            if (After.count(ed)) {
                TerminalState[ids2]=Get_Terminal_type(ed);
            }
            int ids1=find_pos(v);
            // cout<<ids1<<" "<<i<<" "<<ids2<<endl;
            dfa[ids1][i]=ids2;
            After.clear();
        }
    }
}

int get_next_state(int state,char edge) {
    if (edge=='a'||edge=='b'||edge=='c'||edge=='d'||edge=='f'||edge=='k') {
        return dfa[state].count('b')?dfa[state]['b']:dfa[state]['$'];
    }
    // cout<<state<<" "<<edge<<endl;
    if (isdigit(edge)) {
        if (dfa[state].count('a'))return dfa[state]['a'];
    }
    if (isalpha(edge)) {
        if (dfa[state].count('b'))return dfa[state]['b'];
    }
    if (Delimiter.count(edge)) {
        if (dfa[state].count('d'))return dfa[state]['d'];
    }
    if (FilterWord.count(edge)) {
        if (dfa[state].count('c'))return dfa[state]['c'];
    }
    if (edge=='\n') {
        if (dfa[state].count('f'))return dfa[state]['f'];
    }
    if (edge=='@'||!isascii(edge)) {
        if (dfa[state].count('k'))return dfa[state]['k'];
    }
    // cout<<dfa[state].count(edge)<<' '<<edge<<endl;
    return (dfa[state].count(edge)?dfa[state][edge]:dfa[state]['$']);
}
char Convert(char edge) {
    if (isdigit(edge)) {return 'a';}
    if (isalpha(edge)){return 'b';}
    if (FilterWord.count(edge)) {return 'c';}
    if (Delimiter.count(edge)) {return 'd';}
    if (edge=='\n') {return 'f';}
    if (edge=='@'||!isascii(edge)) {return 'k';}
    return edge;
}
bool CheckNextState(int state,char edge) {
    if (edge=='a'||edge=='b'||edge=='c'||edge=='d'||edge=='f'||edge=='k') {
        return dfa[state].count('$')==0&&dfa[state].count('b')==0;
    }
    return dfa[state].count('$')==0&&dfa[state].count(Convert(ch))==0&&dfa[state].count(ch)==0;
}
void ShowCurPointLocation() {
    cout<<"line: "<<line_number<<endl;
    cout<<"start: "<<start_point<<endl;
    cout<<"cur: "<<cur_point<<endl;
}
void EraseLeftSpace(std::string& str) {
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}
void CheckTerminalCode(const int TerminalCode) {
    // cout<<"stop: "<<TerminalCode<<endl;
    switch (TerminalCode) {
        case token_keyword:
            // cout<<" keyword "<<endl;
                break;
        case token_identifier:
            // cout<<" identifier "<<endl;
                break;
        case token_number:
            // cout<<" number "<<endl;
                break;
        case token_operator:
            // cout<<" operator "<<endl;
                break;
        case token_delimiter:
            // cout<<" delimiter "<<endl;
                break;
        case error_number:break;
        case error_identifier:break;
        case error_special:
            cout<<" error_special "<<' '<<Word.size()<<' '<<Word<<endl;
            ShowCurPointLocation();
        break;
        case comment:
            // cout<<" ZhuShi finish "<<endl;
                // cout<<Word<<endl;
                    break;
        case TerminalFlag:
            cout<<" terminal finish "<<endl;
        cout<<Word<<endl;
        break;
        case pre_compilation:
            // cout<<" pre_compilation "<<endl;
                // cout<<Word<<endl;
                    break;
        default:
            cout<<" unknown error! "<<endl;
        cout<<Word<<' '<<TerminalCode<<endl;
        break;
    }
}
void scan() {
    int state=0;
    bool ScanEnd=false;
    while (true) {
        if (file.peek()==EOF) {
            ScanEnd=true;
            ch='@';//完全不影响！
        }
        if (!ScanEnd)ch=file.peek();
        // cout<<ch<<endl;
        if (CheckNextState(state,ch)) {
            if (!~TerminalState[state]) {
                cout<<" error at some DFA! "<<endl;
                ShowCurPointLocation();
                cout<<Word<<' '<<state<<endl;
                state=0;
                Word.clear();
                // if (!ScanEnd)MovePoint();//todo:这个在完整的DFA建立完成后要删掉！
            }
            else {
                EraseLeftSpace(Word);

                int TerminalCode=TerminalState[state];
                if (TerminalCode==token_identifier) {
                    if (KeyWord.count(Word))TerminalCode=token_keyword;
                    // ShowCurPointLocation();
                }

                CheckTerminalCode(TerminalCode);

                if (TerminalCode<=5) {
                    auto token1=Token(TerminalCode,Word);
                    tokens.emplace_back(token1);
                }

                state=0;
                Word.clear();
            }
        }
        else {
            int next_state=get_next_state(state,ch);
            // cout<<state<<' '<<next_state<<endl;
            state=next_state;
            if (!ScanEnd)MovePoint();
        }


        if (ScanEnd){cout<<" finish scan! "<<endl;break;}
    }
}


void handlerError(const string& ErrorText) {
    cout<<"error: "<<ErrorText<<endl;
}
void MovePoint() {
    if (file.peek()==EOF)return;
    file.get(ch);
    if (ch=='\n') {
        line_number++;
        cur_point=start_point=0;
    }
    else {
        EraseLeftSpace(Word);
        if (Word.empty())start_point=cur_point;
        if (ch!='@')Word+=ch;

        cur_point++;
        // cout<<start_point<<' '<<cur_point<<endl;
    }
}