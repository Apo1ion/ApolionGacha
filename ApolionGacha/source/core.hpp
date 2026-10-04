#pragma once
#include <algorithm>
#include <cmath>
#include <fstream>
#include <filesystem>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
namespace gacha {
namespace fs=std::filesystem;
inline std::string trim(std::string s){auto a=s.find_first_not_of(" \t\r\n");if(a==s.npos)return "";auto b=s.find_last_not_of(" \t\r\n");return s.substr(a,b-a+1);}
inline std::vector<std::string> split(const std::string&s,char sep){std::vector<std::string>v;std::stringstream in(s);std::string t;while(std::getline(in,t,sep)){t=trim(t);if(!t.empty()&&std::find(v.begin(),v.end(),t)==v.end())v.push_back(t);}return v;}
inline double number(const std::string&s){if(s.empty()||s.find_first_not_of("0123456789.")!=s.npos||std::count(s.begin(),s.end(),'.')>1)throw std::runtime_error("требуется число через точку, например 6.5");size_t n;double x=std::stod(s,&n);if(n!=s.size()||!std::isfinite(x)||x<0||x>10)throw std::runtime_error("число должно быть от 0.0 до 10.0");return x;}
inline std::string decimal(double x){std::ostringstream o;o.imbue(std::locale::classic());o.precision(8);o<<x;auto s=o.str();if(s.find('.')==s.npos)s+=".0";return s;}
struct Entry {std::string id,name,category,source,fandom,description;double rarity=0;bool enabled=true;};
struct Preset {std::string name;double lo,avg,hi;};
struct Filter {std::map<std::string,int> state;bool permits(const std::vector<std::string>&tags) const {bool any=false,match=false;for(auto&p:state)if(p.second==1)any=true;for(auto&t:tags){auto it=state.find(t);if(it==state.end())continue;if(it->second==-1)return false;if(it->second==1)match=true;}return !any||match;} };
struct Filters {Filter sources,fandoms,categories;bool permits(const Entry&e)const{return sources.permits({e.source})&&fandoms.permits({e.fandom})&&categories.permits({e.category});}};
struct Block {std::map<std::string,std::string>fields;std::string description;int line=0,codeLine=-1;};
struct Parsed {std::vector<Block>blocks;std::vector<std::string>lines,errors;};
inline Parsed parse(const std::string&text,const std::string&start){Parsed p;std::istringstream in(text);std::string line;bool active=false,desc=false;Block b;int n=0;while(std::getline(in,line)){++n;if(n==1&&line.substr(0,3)=="\xef\xbb\xbf")line.erase(0,3);if(!line.empty()&&line.back()=='\r')line.pop_back();p.lines.push_back(line);std::string t=trim(line);if(t==start){if(active)p.errors.push_back("строка "+std::to_string(n)+": предыдущая запись не закрыта");active=true;desc=false;b=Block();b.line=n;continue;}if(t=="=== КОНЕЦ ==="){if(!active){p.errors.push_back("строка "+std::to_string(n)+": лишний конец записи");continue;}b.description=trim(b.description);p.blocks.push_back(b);active=false;desc=false;continue;}if(!active){if(!t.empty()&&t[0]!='#')p.errors.push_back("строка "+std::to_string(n)+": текст вне записи");continue;}if(desc){b.description+=line+"\n";continue;}if(t.empty()||t[0]=='#')continue;auto k=t.find(':');if(k==t.npos){p.errors.push_back("строка "+std::to_string(n)+": ожидалось поле с двоеточием");continue;}auto key=trim(t.substr(0,k)),val=trim(t.substr(k+1));if(b.fields.count(key))p.errors.push_back("строка "+std::to_string(n)+": поле повторяется: "+key);b.fields[key]=val;if(key=="Код")b.codeLine=n-1;if(key=="Описание"){desc=true;if(!val.empty())b.description=val+"\n";}}
if(active)p.errors.push_back("строка "+std::to_string(b.line)+": нет === КОНЕЦ ===");return p;}
inline std::string required(const Block&b,const std::string&key){auto it=b.fields.find(key);if(it==b.fields.end()||it->second.empty())throw std::runtime_error("не заполнено поле «"+key+"»");return it->second;}
inline void keys(const Block&b,const std::set<std::string>&allowed){for(auto&x:b.fields)if(!allowed.count(x.first))throw std::runtime_error("неизвестное поле «"+x.first+"»");}
inline Entry makeEntry(const Block&b,const std::string&cat){keys(b,{"Код","Название","Редкость","Источник","Фэндом","Фэндомы","Включено","Описание"});Entry e;e.category=cat;e.id=required(b,"Код");e.name=required(b,"Название");e.source=required(b,"Источник");if(e.source.find(';')!=e.source.npos)throw std::runtime_error("у записи должен быть ровно один источник");if(b.fields.count("Фэндом")&&b.fields.count("Фэндомы"))throw std::runtime_error("оставьте только одно поле Фэндом");e.fandom=required(b,b.fields.count("Фэндомы")?"Фэндомы":"Фэндом");if(e.fandom.find(';')!=e.fandom.npos)throw std::runtime_error("у записи должен быть ровно один фэндом; версии из разных фэндомов оформляйте отдельно");e.rarity=number(required(b,"Редкость"));auto a=required(b,"Включено");if(a!="Да"&&a!="Нет")throw std::runtime_error("Включено: укажите Да или Нет");e.enabled=a=="Да";e.description=b.description;if(e.description.empty())throw std::runtime_error("описание пустое");return e;}
inline Preset makePreset(const Block&b){keys(b,{"Название","Мин","Сред","Макс"});Preset p{required(b,"Название"),number(required(b,"Мин")),number(required(b,"Сред")),number(required(b,"Макс"))};if(p.lo>p.avg||p.avg>p.hi)throw std::runtime_error("требуется Мин ≤ Сред ≤ Макс");return p;}
struct Pool {std::vector<size_t>indices;std::vector<double>weights;double total=0;};
inline Pool pool(const std::vector<Entry>&entries,const Filters&f,double lo,double avg,double hi,bool all){if(!std::isfinite(lo)||!std::isfinite(avg)||!std::isfinite(hi)||lo<0||hi>10||lo>avg||avg>hi)throw std::runtime_error("Проверьте значения: 0 ≤ Мин. ≤ Сред. ≤ Макс. ≤ 10.");Pool p;for(size_t i=0;i<entries.size();++i){auto&e=entries[i];if(!e.enabled||e.rarity<lo||e.rarity>hi||(!all&&!f.permits(e)))continue;double w=std::pow(4.,-std::abs(avg-e.rarity));p.indices.push_back(i);p.weights.push_back(w);p.total+=w;}return p;}
inline size_t pick(const Pool&p,double u){if(p.indices.empty())throw std::runtime_error("Нет подходящих записей. Измените фильтры или границы редкости.");double target=std::clamp(u,0.,std::nextafter(1.,0.))*p.total;for(size_t j=0;j<p.indices.size();++j){target-=p.weights[j];if(target<0)return j;}return p.indices.size()-1;}
inline std::string read(const fs::path&p){std::ifstream f(p,std::ios::binary);if(!f)throw std::runtime_error("не удалось открыть файл");std::ostringstream s;s<<f.rdbuf();if(f.bad())throw std::runtime_error("ошибка чтения файла");return s.str();}
inline const std::vector<std::string> categories={"Предметы","Способности","Существа","Особенности","Навыки"};
inline const std::vector<std::string> ranks={"Мусорный","Обычный","Необычный","Редкий","Элитный","Эпический","Легендарный","Мифический","Божественный","Трансцендентный"};
inline int tier(double x){return std::clamp((int)std::floor(x),0,9);}
}
