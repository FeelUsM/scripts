#define ONE_SOURCE
#include <strstr/strin.h>
#include <iostream>
//#include <utime.h>
#include <map>
#include <string>
#include <stdlib.h>
#include <algorithm>

using std::string;
using std::pair;
using std::make_pair;
using std::multimap;
using std::cerr;
using std::cout;
using std::endl;
using std::sort;

using str::strin;
using str::read_start_line;
using str::dec;
using str::until_charclass;
using str::spn_crlf;
using str::linecol;

//удаляет все, что не повторяется
template<class cont_t, typename comp_t>
void antiuniq(cont_t * pset, const comp_t & comp)
{
	typedef typename cont_t::iterator it_t;
	if(pset->begin()==pset->end())
		return;
	it_t l,r;
	l=r=pset->begin();
	r++;
	bool fl=false;//в первой итерации первый и (не существующий) минус первый элементы НЕ равны
	while(r!=pset->end()){
		if(comp(*l,*r))//если равны
			fl=true;//в будущем - были равны
		else{//если не равны
			if(!fl){//и не были равны
				pset->erase(l);//удаляем того, кто не равен ни левому ни правому
			}
			fl=false;//в будущем они не были равны
		}
		l=r;
		r++;
	}
	if(!fl)//если последний и предпоследний(может не существовать) элементы не равны
		pset->erase(l);
}
template<class cont_t>
void antiuniq(cont_t * pset){
	antiuniq(pset, [](const typename cont_t::value_type & l, const typename cont_t::value_type & r){return l==r;});
}

int main(){
	multimap<pair<long long,long long>,string> data;
	read_start_line(strin);
	while(!atend(strin)){
		string path;
		long long size, date;
		#define ifnot(expr)	if(!(expr))
		ifnot(read(strin)>>dec(&size)>>'\t'>>dec(&date)
						>>'\t'>>until_charclass(spn_crlf,&path)){
			cerr<<"на позиции "<<linecol(strin)<<" произошла ошибка"<<endl;
			exit(2);
		}
		data.insert(make_pair(make_pair(size,date),move(path)));
		read_start_line(strin);
	}
	antiuniq(&data,
		[](const pair<pair<long long,long long>,string> & l, const pair<pair<long long,long long>,string> & r){
			return l.first.first==r.first.first && l.first.second==r.first.second;//size, date
		}
	);
	
	auto oldit = data.end();
	for(auto it = data.begin(); it!=data.end(); it++){
		if(oldit!=data.end() && (oldit->first.first!=it->first.first || oldit->first.second!=it->first.second))
			cout<<endl;
		cout<<it->first.first<<'\t'<<it->first.second<<'\t'<<it->second<<endl;
		oldit=it;
	}
}