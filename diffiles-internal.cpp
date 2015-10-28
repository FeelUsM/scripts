#include <iostream>
#define one_source
#include "../projs/common-parse-lib/strin.h"
#include <vector>
#include <algorithm>
#include <string>
#include <stdlib.h>
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
			*next = move(zero_value);
		}
		*first = move(zero_value);
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

ostream & operator<<(ostream & str, const pair<pair<char,string>,pair<int,int>> & it);
ostream & operator<<(ostream & str, const vector<pair<pair<char,string>,pair<int,int>>> & );
void print_changed(ostream & str, const vector<pair<pair<char,string>,pair<int,int>>> & );
void print_moved(ostream & str, const vector<pair<pair<char,string>,pair<int,int>>> & );
void print_delited_added(ostream & str, const vector<pair<pair<char,string>,pair<int,int>>> & );
template <class it_t>
void read_diff(it_t & strin, vector<pair<pair<char,string>,pair<int,int>>> * diff, bool inverse);

int main(int argc, const char * argv[])
{
	bool inverse=(argc==2 && argv[1][0]=='-' && argv[1][1]=='i' && argv[1][2]==0);
	vector<pair<pair<char,string>,pair<int,int>>> diff;//-+,path , size,date
	read_diff(strin,&diff,inverse);
	
	std::sort(diff.begin(),diff.end(),
		[](const pair<pair<char,string>,pair<int,int>> & l, const pair<pair<char,string>,pair<int,int>> & r){
			return l.first.second<r.first.second || l.first.second==r.first.second && l.first.first>r.first.first;
			//path, -+
	});
	//cout<<"=== before changed === "<<endl<<diff<<endl;
	vector<pair<pair<char,string>,pair<int,int>>> changed;//-+,path , size,date
	auto end1 = antiuniq_copy_remove(diff.begin(),diff.end(),back_inserter(changed),
		move(make_pair(move(make_pair('\0',move(string("")))),move(make_pair(0,0)))),
		[](const pair<pair<char,string>,pair<int,int>> & l, const pair<pair<char,string>,pair<int,int>> & r){
			return l.first.second==r.first.second;
			//path
	});
	print_changed(cout,changed);
	size_t s=end1-diff.begin();
	while(diff.size()>s)	diff.pop_back();

	std::sort(diff.begin(),diff.end(),
		[](const pair<pair<char,string>,pair<int,int>> & l, const pair<pair<char,string>,pair<int,int>> & r){
			return l.second.first<r.second.first || l.second.first==r.second.first && l.second.second<r.second.second
			|| l.second.first==r.second.first && l.second.second==r.second.second && l.first.first>r.first.first;
			//size, date, -+
	});
	
	//cout<<"=== before moved === "<<endl<<diff<<endl;
	changed.clear();
	end1 = antiuniq_copy_remove(diff.begin(),diff.end(),back_inserter(changed),
		move(make_pair(move(make_pair('\0',move(string("")))),move(make_pair(0,0)))),
		[](const pair<pair<char,string>,pair<int,int>> & l, const pair<pair<char,string>,pair<int,int>> & r){
			return l.second.first==r.second.first && l.second.second==r.second.second;
			//size, date
	});
	s=end1-diff.begin();
	while(diff.size()>s)	diff.pop_back();
	end1 = antiuniq_copy_remove(changed.begin(),changed.end(),back_inserter(diff),
		move(make_pair(move(make_pair('\0',move(string("")))),move(make_pair(0,0)))),
		[](const pair<pair<char,string>,pair<int,int>> & l, const pair<pair<char,string>,pair<int,int>> & r){
			return l.second.first==r.second.first && l.second.second==r.second.second && l.first.first==r.first.first;
			//size, date, +-
		}
	);
	s=end1-changed.begin();
	while(changed.size()>s)	changed.pop_back();
	print_moved(cout,changed);
	
	std::sort(diff.begin(),diff.end(),
		[](const pair<pair<char,string>,pair<int,int>> & l, const pair<pair<char,string>,pair<int,int>> & r){
			return l.first.first>r.first.first || l.first.first==r.first.first && l.first.second<r.first.second;
			//-+, path
	});
	print_delited_added(cout,diff);
}
ostream & operator<<(ostream & str, const pair<pair<char,string>,pair<int,int>> & it){
	return str<<it.first.first<<it.second.first<<'\t'<<it.second.second<<'\t'<<it.first.second;
}
ostream & operator<<(ostream & str, const vector<pair<pair<char,string>,pair<int,int>>> & vec){
	for(auto it=vec.begin(); it!=vec.end(); it++)
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
		if(*s=='\"' || *s=='\\' || *s=='$')
			str<<'\\';
		str <<*s++;
	}
	return str;
}

void print_changed(ostream & str, const vector<pair<pair<char,string>,pair<int,int>>> & vec){
	if(vec.empty())	return;
	cout<<"# === changed ==="<<endl;//<<vec<<endl;
	for(auto it=vec.begin(); it!=vec.end(); it++){
		if(it->first.first!='-'){
			cerr<<"diffiles: ERROR: changed: expected file '-'"<<endl<<*it<<endl;
			exit(1);
		}
		const char * sss = it->first.second.c_str();
		int old_size = it->second.first;
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
			<<"#from "<<old_size<<" to "<<it->second.first<<" at "<<it->second.second<<endl;
	}
}
void print_moved(ostream & str, const vector<pair<pair<char,string>,pair<int,int>>> & vec){
	if(vec.empty())	return;
	vector<pair<pair<char,string>,pair<int,int>>> undetected;
	str<<"#=== moved ==="<<endl;
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
			pair<int,int> sss = it->second;
			const char * name = it->first.second.c_str();
			it++;
			if(it->first.first!='+')
				goto met;
			if(it->second.first!=sss.first || it->second.second!=sss.second)
				goto met;
			str	<<"git mv \""<<quote_out(name)<<"\" \""<<quote_out(it->first.second.c_str())<<'"'
				<<" # "<<it->second.first<<endl;
		}
		first=next;
	}

	if(undetected.empty())	return;
	str<<"#=== UNDETECTED MOVES ==="<<endl;//<<vec<<endl;
	for(auto it=undetected.begin(); it!= undetected.end(); it++)
		str<<"#"<<*it<<endl;
}
void print_delited_added(ostream & str, const vector<pair<pair<char,string>,pair<int,int>>> & vec){
	if(vec.empty())	return;
	auto it=vec.begin();
	if(it->first.first=='-'){
		cout<<"#=== deleted ==="<<endl;//<<vec<<endl;
		for(; it!=vec.end() && it->first.first=='-'; it++)
			str <<"git rm \""<<quote_out(it->first.second.c_str())<<'"'
				<<" # "<<it->second.first<<endl;
	}
	if(it==vec.end()) return;
	str<<endl<<"#=== added ==="<<endl;
	for(; it!=vec.end() && it->first.first=='+'; it++)
		str <<"git add \""<<quote_out(it->first.second.c_str())<<'"'
			<<" # "<<it->second.first<<" at "<<it->second.second<<endl;
}

template <class it_t>
void read_diff(it_t & strin, vector<pair<pair<char,string>,pair<int,int>>> * diff, bool inverse){
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
		int size, date;
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
