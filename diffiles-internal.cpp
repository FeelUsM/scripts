#include <iostream>
#define ONE_SOURCE
#include <strstr/strin.h>
#include <vector>
#include <algorithm>
#include <string>
#include <stdlib.h>
#include <time.h>
using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::ostream;
using std::vector;
using std::pair;
using std::make_pair;
using std::move;

using str::strin;
using str::read_fix_str;
using str::read_line;
using str::read_start_line;
using str::linecol;
using str::ch;
using str::dec;
using str::spn_crlf;
using str::until_charclass;
using str::make_span;

//находит соседние повторяющиеся
//и если выполняется pred2 от первого и последнего из них
//	заменят их на zero_value
//а потом удаляет его
template<class init_t, class outit_t, typename bin_pred_t, typename bin_pred_t2, class val_t>
init_t antiuniq_copy_remove_check(init_t first, init_t last, outit_t out, 
	const val_t & zero_value, const bin_pred_t & pred, const bin_pred_t2 & pred2)
{
	auto first2 = first;
	while((first = adjacent_find(first,last,pred))!=last){
		auto next = first;
		//cerr<<"group"<<endl;
		for(; next!=last && pred(*first,*next); next++)
			//cerr<<*next<<endl;
			;
		//cout <<"compare("<<next-first<<"):"<<endl<<*first<<endl<<*prev(next)<<endl;
		
		if(pred2(*first,*prev(next))){
			//cout<<"do this"<<endl;
			for(auto it = first; it!=next; it++){
				*out++ = move(*it);
				*it = zero_value;
			}
		}
		else
			;//cout <<"false"<<endl;
		first=next;
	}
	return remove(first2,last,zero_value);
}
//копирует соседние повторяющиеся
//заменят их на zero_value
//а потом удаляет его
template<class init_t, class outit_t, typename bin_pred_t, class val_t>
init_t antiuniq_copy_remove(init_t first, init_t last, outit_t out, 
	const val_t & zero_value, const bin_pred_t & pred)
{
	auto first2 = first;
	while((first = adjacent_find(first,last,pred))!=last){
		auto next = first;
		*out++ = (*next);
		for(next++; next!=last && pred(*first,*next); next++){
			*out++ = move(*next);
			*next = zero_value;
		}
		*first = zero_value;
		first=next;
	}
	return remove(first2,last,zero_value);
}
template<class init_t, class outit_t, class val_t>
init_t antiuniq_copy_remove(init_t first, init_t last, outit_t out, const val_t & zero_value)
{
	return antiuniq_copy(first,last,out,zero_value,[](const val_t & l, const val_t & r){return l==r;});
}

/*
+-, размер,дата,путь
отсортировать, вывести повторяющиеся, и удалить
по дате/размеру
	вывести пары -+ и найти общие папки
	вывести прочее и найти общие папки
отсортировать, вывести повторяющиеся, и удалить
по имени
	проверить пары -+
вывести все -
вывести все +

todo:
сделать вывод размеров и дат
при выводе undetected: если все - или все +, то их в создание/удаление
*/

ostream & operator<<(ostream & str, const pair<pair<char,string>,pair<long long,int>> & it);
ostream & operator<<(ostream & str, const vector<pair<pair<char,string>,pair<long long,int>>> & );
void print_changed(ostream & str, const vector<pair<pair<char,string>,pair<long long,int>>> & );
void print_moved(ostream & str, const vector<pair<pair<char,string>,pair<long long,int>>> & );
void print_delited_added(ostream & str, const vector<pair<pair<char,string>,pair<long long,int>>> & );
template <class it_t>
void read_diff(it_t & strin, vector<pair<pair<char,string>,pair<long long,int>>> * diff, bool inverse);

// === MAIN ===
int main(int argc, const char * argv[])
{
	bool inverse=(argc==2 && argv[1][0]=='-' && argv[1][1]=='i' && argv[1][2]==0);
	vector<pair<pair<char,string>,pair<long long,int>>> diff;//-+,path , size,date
	read_diff(strin,&diff,inverse);

	//=== changed ===
	std::sort(diff.begin(),diff.end(),
		[](const pair<pair<char,string>,pair<long long,int>> & l, const pair<pair<char,string>,pair<long long,int>> & r){
			return l.first.second<r.first.second || l.first.second==r.first.second && l.first.first>r.first.first;
			//path, -+
	});
	//cout<<"=== before changed === "<<endl<<diff<<endl;
	vector<pair<pair<char,string>,pair<long long,int>>> changed;//-+,path , size,date
	auto end1 = antiuniq_copy_remove(diff.begin(),diff.end(),back_inserter(changed),
		move(make_pair(move(make_pair('\0',move(string("")))),move(make_pair((long long)0,0)))),
		[](const pair<pair<char,string>,pair<long long,int>> & l, const pair<pair<char,string>,pair<long long,int>> & r){
			return l.first.second==r.first.second;
			//path
	});	diff.erase(end1,diff.end());
	print_changed(cout,changed);
	changed.clear();

	//=== moved ===
	std::sort(diff.begin(),diff.end(),
		[](const pair<pair<char,string>,pair<long long,int>> & l, const pair<pair<char,string>,pair<long long,int>> & r){
			return l.second.first<r.second.first || l.second.first==r.second.first && l.second.second<r.second.second
			|| l.second.first==r.second.first && l.second.second==r.second.second && l.first.first>r.first.first;
			//size, date, -+
	});
	//cout<<"=== before moved === "<<endl<<diff<<endl;
	//перемещенные - с одинаковым размером и временем изменения 
	//и если среди них присутствуют различные знаки
	end1 = antiuniq_copy_remove_check(diff.begin(),diff.end(),back_inserter(changed),
		move(make_pair(move(make_pair('\0',move(string("")))),move(make_pair((long long)0,0)))),
		[](const pair<pair<char,string>,pair<long long,int>> & l, const pair<pair<char,string>,pair<long long,int>> & r){
			return l.second.first==r.second.first && l.second.second==r.second.second;
			//size, date
		},
		[](const pair<pair<char,string>,pair<long long,int>> & l, const pair<pair<char,string>,pair<long long,int>> & r){
			return l.first.first!=r.first.first;
			//+-
		}
	);	diff.erase(end1,diff.end());
	print_moved(cout,changed);
	
	//=== deleted/added ===
	std::sort(diff.begin(),diff.end(),
		[](const pair<pair<char,string>,pair<long long,int>> & l, const pair<pair<char,string>,pair<long long,int>> & r){
			return l.first.first>r.first.first || l.first.first==r.first.first && l.first.second<r.first.second;
			//-+, path
	});
	print_delited_added(cout,diff);
}

ostream & operator<<(ostream & str, const pair<pair<char,string>,pair<long long,int>> & it){
	return str<<it.first.first<<it.second.first<<'\t'<<it.second.second<<'\t'<<it.first.second;
}
ostream & operator<<(ostream & str, const vector<pair<pair<char,string>,pair<long long,int>>> & vec){
	for(auto it=vec.cbegin(); it!=vec.cend(); it++)
		str<<*it<<endl;
	return str;
}
struct quote_out{
	const char * s;
quote_out(const char * ss):s(ss){}
};
std::ostream & operator<<(std::ostream & str, quote_out q){
	const char * s=q.s;
	while(*s){
		if(*s=='\"' || *s=='\\' || *s=='$' || *s=='`')
			str<<'\\';
		str <<*s++;
	}
	return str;
}

struct datetime{
	time_t tt;
	datetime(time_t tttt):tt(tttt){}
};
std::ostream & operator<<(std::ostream & str, datetime dt){
	tm * ptime = localtime(&dt.tt);
	using namespace std;
	return str<<setfill('0')<<setw(2)<<(ptime->tm_year+1900)<<"-"
		<<setfill('0')<<setw(2)<<(ptime->tm_mon+1)<<"-"
		<<setfill('0')<<setw(2)<<(ptime->tm_mday)<<" "
		<<setfill('0')<<setw(2)<<ptime->tm_hour<<"-"
		<<setfill('0')<<setw(2)<<ptime->tm_min<<"-"
		<<setfill('0')<<setw(2)<<ptime->tm_sec;
}

struct bytes{
	long long b;
	bytes(long long x):b(x){}
};
std::ostream & operator<<(std::ostream & str, bytes bb){
	int x;
	if(x=bb.b/1024/1024/1024)
		str<<x<<"GB ";
	bb.b%=(1024*1024*1024);
	if(x=bb.b/1024/1024)
		str<<x<<"MB ";
	bb.b%=(1024*1024);
	if(x=bb.b/1024)
		str<<x<<"kB ";
	bb.b%=(1024);
	if(x=bb.b)
		str<<x<<"B";
	return str;
}

//ожидает список -+-+-+... и чтобы у пар совпадали имена
void print_changed(ostream & str, const vector<pair<pair<char,string>,pair<long long,int>>> & vec){
	cout<<"# === changed ==="<<endl;//<<vec<<endl;
	if(vec.empty())	return;
	for(auto it=vec.begin(); it!=vec.end(); it++){
		if(it->first.first!='-'){
			cerr<<"diffiles: ERROR: changed: expected file '-'"<<endl<<*it<<endl;
			exit(1);
		}
		const char * sss = it->first.second.c_str();
		long long old_size = it->second.first;
		it++;
		if(it->first.first!='+'){
			cerr<<"diffiles: ERROR: changed: expected file '+'"<<endl<<*it<<endl;
			exit(1);
		}
		if(it->first.second!=sss){
			cerr<<"diffiles: ERROR: changed: expected file '+' with name "<<sss<<endl<<*it<<endl;
			exit(1);
		}
		str <<"git add \""<<quote_out(it->first.second.c_str())<<"\" "
			<<"#from "<<bytes(old_size)<<" to "<<bytes(it->second.first)<<" at "<<datetime(it->second.second)<<endl;
	}
}
//ожидает список с повторяющимися датой/размером,
//выводит moved и UNDETECED MOVED
void print_moved(ostream & str, const vector<pair<pair<char,string>,pair<long long,int>>> & vec){
	str<<"#=== moved ==="<<endl;
	if(vec.empty())	return;
	vector<pair<pair<char,string>,pair<long long,int>>> undetected;
	auto first = vec.begin();
	auto last = vec.end();
	while(first!=last){
		auto next = first;
		next++;
		int n=1;
		for(; next!=last && next->second.first==first->second.first && next->second.second==first->second.second; next++)
			n++;
		if(n!=2)
met:
			for(auto it=first; it!=next; it++)
				undetected.push_back(move(*it));
		else{
			auto it = first;
			if(it->first.first!='-')
				goto met;
			pair<long long,int> sss = it->second;
			const char * name = it->first.second.c_str();
			it++;
			if(it->first.first!='+')
				goto met;
			if(it->second.first!=sss.first || it->second.second!=sss.second)
				goto met;
			str	<<"git mv \""<<quote_out(name)<<"\" \""<<quote_out(it->first.second.c_str())<<'"'
				<<" # "<<bytes(it->second.first)<<endl;
		}
		first=next;
	}

	str<<"#=== UNDETECTED MOVES ==="<<endl;//<<vec<<endl;
	if(undetected.empty())	return;
	for(auto it=undetected.begin(); it!= undetected.end(); it++)
		str<<"#"<<*it<<endl;
}
//ожидает сначала список - и выводит deleted, а потом список + и выводит added
void print_delited_added(ostream & str, const vector<pair<pair<char,string>,pair<long long,int>>> & vec){
	cout<<"#=== deleted ==="<<endl;//<<vec<<endl;
	if(vec.empty())	return;
	auto it=vec.begin();
	if(it->first.first=='-'){
		for(; it!=vec.end() && it->first.first=='-'; it++)
			str <<"git rm \""<<quote_out(it->first.second.c_str())<<'"'
				<<" # "<<bytes(it->second.first)<<" at "<<datetime(it->second.second)<<endl;
	}
	if(it==vec.end()) return;
	str<<endl<<"#=== added ==="<<endl;
	for(; it!=vec.end() && it->first.first=='+'; it++)
		str <<"git add \""<<quote_out(it->first.second.c_str())<<'"'
			<<" # "<<bytes(it->second.first)<<" at "<<datetime(it->second.second)<<endl;
}

template <class it_t>
void read_diff(it_t & strin, vector<pair<pair<char,string>,pair<long long,int>>> * diff, bool inverse){
	read_start_line(strin);
	if(!read_fix_str(strin,"---")){
		cerr<<"на позиции "<<linecol(strin)<<" ожидалось '---'"<<endl;
		exit(1);
	}
	read_line(strin);
	read_start_line(strin);
	if(!read_fix_str(strin,"+++")){
		cerr<<"на позиции "<<linecol(strin)<<" ожидалось '+++'"<<endl;
		exit(1);
	}
	read_line(strin);
	read_start_line(strin);
	
	while(!atend(strin)){
		if(read_fix_str(strin,"@@")){
			read_line(strin);
			read_start_line(strin);
			continue;
		}
		char c;
		string path;
		long long size;
		int date;
		#define ifnot(expr)	if(!(expr))
		ifnot(read(strin)>>charclass_c(make_span("+-"),&c)>>dec(&size)>>'\t'>>dec(&date)
						>>'\t'>>until_charclass(spn_crlf,&path)){
			cerr<<"на позиции "<<linecol(strin)<<" произошла ошибка"<<endl;
			exit(1);
		}
		if(inverse)
			c= c=='+'?'-':'+';
		diff->push_back(move(make_pair(move(make_pair(c,move(path))),make_pair(size,date))));
		read_start_line(strin);
	}
}
