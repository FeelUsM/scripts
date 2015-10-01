#include <iostream>
#include <fstream>
#define one_source
#include "../projs/common-parse-lib/strin.h"
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <utility>//pair
#include <iterator>//iterator_traits,inserters,movers
#include <limits.h>
#include <iomanip>
#include <set>
using std::cerr;
using std::cout;
using std::endl;
using namespace str;
using std::pair;
using std::make_pair;
using std::string;
using std::vector;
using std::multimap;
using std::map;
using std::set;

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

//удаляет все, что не повторяется
template<class init_t, class outit_t, typename comp_t>
outit_t antiuniq_copy(init_t first, init_t last, outit_t out, const comp_t & comp)
{
	if(first==last) return out;
	init_t l = first;	first++;
	bool f1 = false;
	while(first!=last){
		if(comp(*l,*first)){
			if(!f1){
				*out++ = *l;
				f1=true;
			}
			*out++ = *first;
		}
		else
			f1 = false;
		l++;
		first++;
	}
	return out;
}
template<class init_t, class outit_t>
outit_t antiuniq_copy(init_t first, init_t last, outit_t out)
{
	typedef decltype(*first) val_t;
	return antiuniq_copy(first,last,out,[](const val_t & l, const val_t & r){return l==r;});
}

//работа с экранированными и неэкранированными строками
char * quote_strcpy(char * to, const char * from){
	while(*from){
		if(*from=='"')
			*to++='\\';
		*to++=*from++;
	}
	*to='\0';
	return to;
}
size_t quote_strlen(const char * from){
	size_t rez=strlen(from);
	while(from=strchr(from,'"'))
		rez++, from++;
	return rez;
}

template <class it_t, class mes_t>
void dump(it_t tmp, mes_t mes){
	string s;
	read_fix_length(tmp,50,&s);
	cerr <<mes <<dump(s.c_str()) <<endl;
	//это не рекурсия, это в forward_stream.h определено для дебага, и здесь тоже пригодилось
}

struct bytes{
	int size;
	bytes(int x):size(x){}
};
std::ostream & operator<<(std::ostream & str, bytes b){
	double size2=b.size;
	if(size2/1024<1.1)
		return str<<b.size<<"B";
	size2/=1024;
	if(size2/1024<1.1)
		return str<<std::setprecision(4)<<size2<<"kB";
	size2/=1024;
	if(size2/1024<1.1)
		return str<<std::setprecision(4)<<size2<<"MB";
	size2/=1024;
	if(size2/1024<1.1)
		return str<<std::setprecision(4)<<size2<<"GB";
	return str<<std::setprecision(4)<<size2<<"GB";
}

pair<const char *, const char *> str_mismatch(const char * s1, const char *s2){
	while(*s1 && *s2 && *s1==*s2)
		s1++, s2++;
	return make_pair(s1,s2);
}

const char * find_ext(const string * ps){
	int dirpos=ps->rfind('/'), extpos=ps->rfind('.');
	if(extpos>=0 && dirpos<extpos)
		return ps->c_str()+extpos;
	else
		return "";
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

/*
из <limits.h> получить ARG_MAX, из environ??? получить размер среды, вычесть его, вычесть еще 1000, и создать такого размера строку, и в нее дописывать 
env | wc -c
openssl md5 "файл" "файл" файл...	

отступление:
	рекурсивное подтягивание хешей из подпапок в текущую папку - отдельная утилитка pull-hashes
	анализ повторов, поиск повторяющихся ПАПОК и др. - отдельная утилитка dups-analys
псевдокод:
	lsdups-internal.cpp файлы хеши > повторы
		повторы:
			\n
			размер
			файл
			файл
			файл
	прочитать файлы (размер дата имя)
	antiuniq<размер>
	прочитать хеши (хеш дата имя)
	выкинуть файлы, у которых уже посчитаны хеши с той же датой
	{
		подготовить строку для вызова
		вызвать и перенаправить к нам
		обработать и добавить в файл с хешами и в наш контейнер
	}
	antiuniq<хеш>
	вывести результат
*/

int main(int argc, const char * argv[]){
	if(argc!=3)
	{	cerr << "usage: sldups-internal files-file hashes-file" << endl; return 1;	}

	//=== считываем файлы (размер дата имя) ===
	typedef pair<string,pair<int,int>> sii_t;
	typedef vector<sii_t> sii_cont;
	sii_cont psd;//path size date
	{
		forward_adressed_stream files(true, new file_on_FILE(argv[1],"r"));
		if(!files){	
			cerr <<"не смог открыть файл "<<argv[1] <<endl;	
			return 1;	
		}
		while(!atend(files.iter())){
			sii_t sii;
			if(!(read_dec(files.iter(),&sii.second.first)
				&&E2F(read_dec(files.iter(),&sii.second.second))
				&&E2F(read_fix_char(files.iter(),'\t'))
				&&E2F(read_until_charclass(files.iter(),spn_crlf,&sii.first).reset())
			))
			{	cerr <<"error in files-file "<<argv[1]<<" at " <<get_linecol(files.iter()) <<endl; return 1;	}
			start_read_line(files.iter());
			psd.push_back(move(sii));
		}
	}
	std::sort(psd.begin(),psd.end(),
		[](const pair<string,pair<int,int>> & l, const pair<string,pair<int,int>> & r){
			return l.second.first<r.second.first;//size
		}
	);
	//=== ищем кандидатов по размеру ===
	map<string,pair<int,int>> psd2;
	antiuniq_copy(make_move_iterator(psd.begin()),make_move_iterator(psd.end()),inserter(psd2,psd2.end()),
		[](const pair<string,pair<int,int>> & l, const pair<string,pair<int,int>> & r){
			return l.second.first==r.second.first;//size
		}
	);
	//=== считываем закешированные хеши (хеш дата имя) ===
	typedef pair<string,pair<string,int>> ssi_t;
	multimap<string,pair<string,int>> hps;//hash path size
	{
		forward_adressed_stream hashes(true, new file_on_FILE(argv[2],"r"));
		if(hashes){	
			while(!atend(hashes.iter())){
				string hash;
				int datetime;
				string path;
				if(!(read_until_char(hashes.iter(),'\t',&hash)
					&&E2F(read_dec(hashes.iter(),&datetime))
					&&E2F(read_fix_char(hashes.iter(),'\t'))
					&&E2F(read_until_charclass(hashes.iter(),spn_crlf,&path).reset())
				))
				{	cerr <<"error in hashes-file at " <<get_linecol(hashes.iter()) <<endl; return 1;	}
				start_read_line(hashes.iter());
				
				auto it = psd2.find(path);
				if(it==psd2.end())	continue;
				if(it->second.second/*date*/ != datetime)	continue;
				hps.insert(move(make_pair(move(hash),make_pair(it->first,it->second.first))));//hash path size
				psd2.erase(it);
			}
		}
		//нет файла - ну и хрен с ним - вычислим
	}	//hashes.close()
	//=== вычисляем НЕзакешированные хеши ===
	//for(auto it=psd2.begin(); it!=psd2.end(); it++)
		//cerr<<it->second.first<<"\t"<<it->first<<endl;
	int comstrlen=ARG_MAX-1000;
	{
		forward_adressed_stream envlen(true, new file_on_FILE(popen("env | wc -c","r")));
		if(!envlen){	cerr<<"не смог выполнить 'env | wc -c'"<<endl;	return 1;	}
		int x;
		if(!read_dec(envlen.iter(),&x)){	cerr<<"не смог прочитать число в 'env | wc -c'"<<endl;	return 1;	}
		read_spcs(envlen.iter());
		if(!atend(envlen.iter())){	cerr<<"после прочтения 'env | wc -c' что-то осталось"<<endl;	return 1;	}
		comstrlen-=x;
	}
	//cerr<<"comstrlen="<<comstrlen<<endl;
	std::ofstream hashes(argv[2],std::ofstream::app);
	if(!hashes){	cerr<<"не смог открыть "<<argv[2]<<endl;	return 1;	}
	while(!psd2.empty())
	{
		char array[comstrlen];
		char * begin = array, * end = array+comstrlen;
		const char * com = "openssl md5";
		strcpy(begin,com);
		begin += strlen(com);
		auto it = psd2.begin();
		while(it!=psd2.end() && (size_t)(end-begin)>quote_strlen(it->first.c_str())+3){
			*begin++=' ';
			*begin++='"';
			begin=quote_strcpy(begin,it->first.c_str());
			*begin++='"';
			*begin='\0';
			it++;
		}
		forward_adressed_stream md5(true,new file_on_FILE(popen(array,"r")));
		if(!md5)	{	cerr<<"не смог вызвать openssl"<<endl;	return 1;	}
		while(read_fix_str(md5.iter(),"MD5(")){
			dump(md5.iter(),"будет прочитано: ");
			while(!read_fix_str(md5.iter(),psd2.begin()->first.c_str())){
				cerr<<"файл "<<psd2.begin()->first<<" не посчитался"<<endl;
				if(it==psd2.begin()){	cerr<<"не можем найти файл"<<endl;	return 1;	}
				psd2.erase(psd2.begin());
			}
			if(!read_fix_str(md5.iter(),")= "))
			{	cerr<<"неправильный синатксис после "<<psd2.begin()->first<<endl;	return 1;	}
			string hash;
			read_until_charclass(md5.iter(),spn_crlf,&hash);
			start_read_line(md5.iter());
			hashes<<hash<<'\t'<<psd2.begin()->second.second<<'\t'<<psd2.begin()->first<<endl;
			hps.insert(move(make_pair(move(hash),make_pair(move(psd2.begin()->first),psd2.begin()->second.first))));
			psd2.erase(psd2.begin());
		}
		while(psd2.begin()!=it){
			cerr<<"файл "<<psd2.begin()->first<<" не посчитался"<<endl;
			psd2.erase(psd2.begin());
		}
		if(!atend(md5.iter()))
		{	cerr<<"ожидался конец файла"<<endl;	}
	}
	cerr <<"итого до хешей: "<<hps.size()<<" файлов"<<endl;
	//for(auto it=hps.begin(); it!=hps.end(); it++)
	//	cout <<it->first<<'\t'<<it->second.second<<'\t'<<it->second.first<<endl;
	antiuniq(&hps,[](const ssi_t & l, const ssi_t & r){	return l.first==r.first;	});
	cerr <<"итого: "<<hps.size()<<" файлов"<<endl;
	
	map<string,map<string,multimap<int,set<string>>>> dups;//comm_path,ext,size,diff_path
	if(0){
		auto sss=hps.begin();
		int size=0;
		cout<<"///"<<hps.begin()->second.second<<endl;
		for(auto it=hps.begin(); it!=hps.end(); it++){
			if(it->first!=sss->first){
				sss=it;
				size-=it->second.second;
				cout<<"///"<<it->second.second<<endl;
			}
			size+=it->second.second;
			cout <<it->second.first.c_str()<<endl;
		}
		cerr<<"итого "<<bytes(size)<<" избыточного"<<endl;
		return 0;
	}
	else{
		auto sss=hps.begin();
		int tot_size=0;
																			//cerr<<"///"<<hps.begin()->second.second<<endl;
																			cerr<<"итого "<<bytes(tot_size)<<" избыточного"<<endl;
		int size= (hps.begin()==hps.end()) ? 0 : hps.begin()->second.second;
		set<string> ldups;//файлы
		for(auto it=hps.begin(); true; it++){
			if(it==hps.end() || it->first!=sss->first){
				sss=it;
				tot_size-=it->second.second;
																			//cerr<<"///"<<it->second.second<<endl;
																			//cerr<<"итого "<<bytes(tot_size)<<" избыточного"<<endl;
				string compath;//общий путь и удаление его из файлов
				{
					auto ff=ldups.begin(), ll=ldups.end();	ll--;
					int comlen=str_mismatch(ff->c_str(),ll->c_str()).first-ff->c_str();
					//cerr <<"from "<<*ff<<endl<<"to "<<*ll<<endl<<"lenis"<<comlen<<endl;
					while(comlen>0 && ff->c_str()[comlen-1]!='/')
						comlen--;
					if(comlen){
						compath.append(ff->c_str(),comlen);
						for(auto it=ldups.begin(); it!=ldups.end(); it++)
							const_cast<string&>(*it).erase(0,comlen);// !!! порядок не нарушится
					}
				}
				
				string comext;//общее расширение
				{
					auto it=ldups.begin();
					comext=find_ext(&*it++);
					for(;it!=ldups.end(); it++)
						if(comext!=find_ext(&*it))
							comext.clear();
				}
				
				dups[compath][comext].insert(move(make_pair(size,move(ldups))));
				
				if(it==hps.end())	break;
				size= it->second.second;
				ldups.clear();
			}
			tot_size+=it->second.second;
																			//cerr <<it->second.first.c_str()<<endl;
			ldups.insert(move(it->second.first));
		}


		cerr<<"итого "<<bytes(tot_size)<<" избыточного"<<endl;
	}
	
	for(auto it_compath=dups.begin(); it_compath!=dups.end(); it_compath++){
		if(!it_compath->first.empty())
			cout 		<<"cd \""<<quote_out(it_compath->first.c_str())<<"\"";
		int size=0;
		for(auto it_comext=it_compath->second.begin(); it_comext!=it_compath->second.end(); it_comext++)
			for(auto it_size=it_comext->second.begin(); it_size!=it_comext->second.end(); it_size++)
				size+=it_size->first*(it_size->second.size()-1);
			cout << " #"<<bytes(size);
		for(auto it_comext=it_compath->second.begin(); it_comext!=it_compath->second.end(); it_comext++){
			int size=0;
			for(auto it_size=it_comext->second.begin(); it_size!=it_comext->second.end(); it_size++)
				size+=it_size->first*(it_size->second.size()-1);
			if(it_comext->first.size()>0)
				cout<<endl<<"#    "<< it_comext->first<<" ("<<bytes(size)<<")"<<endl;
			else
				cout<<endl<<"#    no ext"<<" ("<<bytes(size)<<")"<<endl;
			for(auto it_size=it_comext->second.begin(); it_size!=it_comext->second.end(); it_size++){
				cout	<<"#      "<<it_size->first<<endl;
				for(auto it_path=it_size->second.begin(); it_path!=it_size->second.end(); it_path++)
					cout<<"#rm \"" <<quote_out(it_path->c_str())<<"\""<<endl;
			}
		}
		if(!it_compath->first.empty())
			cout 		<<"cd -"<<endl<<endl;
	}
}
