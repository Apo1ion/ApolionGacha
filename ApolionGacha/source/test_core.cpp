#include "core.hpp"
#include <cassert>
#include <chrono>
#include <iostream>
using namespace gacha;
int main(int argc,char**argv){
 std::string root=argc>1?argv[1]:"ApolionGacha";
 std::vector<Entry> all;for(auto&cat:categories){auto p=parse(read(fs::u8path(root+"/data/"+cat+".txt")),"=== ЗАПИСЬ ===");assert(p.errors.empty());assert(p.blocks.size()==1);all.push_back(makeEntry(p.blocks[0],cat));}
 auto pp=parse(read(fs::u8path(root+"/data/Шаблоны.txt")),"=== ШАБЛОН ===");assert(pp.errors.empty());assert(pp.blocks.size()==8);for(auto&b:pp.blocks)makePreset(b);
 Filters f;auto p=pool(all,f,.1,1.3,3.3,false);assert(p.indices.size()==5);double sum=0;for(auto w:p.weights)sum+=w/p.total;assert(std::abs(sum-1)<1e-12);
 f.categories.state["Предметы"]=1;p=pool(all,f,0,1.3,10,false);assert(p.indices.size()==1&&all[p.indices[0]].category=="Предметы");assert(pool(all,f,0,1.3,10,true).indices.size()==5);
 f.categories.state.clear();f.sources.state["Реальный мир"]=-1;assert(pool(all,f,0,1.3,10,false).indices.size()==2);
 f.sources.state["Авторское"]=1;assert(pool(all,f,0,1.3,10,false).indices.size()==2);
 f=Filters();f.fandoms.state["Авторский мир"]=1;assert(pool(all,f,0,1.3,10,false).indices.size()==2);
 Entry cross=all[1];cross.fandom="A";Filters ff;ff.fandoms.state["A"]=1;assert(ff.permits(cross));ff.fandoms.state["A"]=-1;assert(!ff.permits(cross));ff.fandoms.state["B"]=1;assert(!ff.permits(cross));
 auto block=parse(read(fs::u8path(root+"/data/Предметы.txt")),"=== ЗАПИСЬ ===").blocks[0];block.fields["Фэндом"]="A; B";bool bad=false;try{makeEntry(block,"Предметы");}catch(...){bad=true;}assert(bad);block.fields["Фэндом"]="A";assert(makeEntry(block,"Предметы").fandom=="A");block.fields.erase("Фэндом");block.fields["Фэндомы"]="A";assert(makeEntry(block,"Предметы").fandom=="A");block.fields["Фэндомы"]="A; B";bad=false;try{makeEntry(block,"Предметы");}catch(...){bad=true;}assert(bad);

 f=Filters();p=pool(all,f,1.3,1.3,1.3,false);assert(p.indices.size()==2);for(auto i:p.indices)assert(all[i].rarity==1.3);
 assert(pool(all,f,9,9.5,10,false).indices.empty());bool thrown=false;try{pick(Pool(),.2);}catch(...){thrown=true;}assert(thrown);
 for(auto args:std::vector<std::vector<double>>{{5,2,8},{2,8,5},{-1,2,5},{0,2,11}}){thrown=false;try{pool(all,f,args[0],args[1],args[2],false);}catch(...){thrown=true;}assert(thrown);}
 for(auto s:{"6,5","nan","inf","11","-1","1..2",""}){thrown=false;try{number(s);}catch(...){thrown=true;}assert(thrown);}assert(number("6.5")==6.5);
 auto broken=parse("=== ЗАПИСЬ ===\nНазвание: A\nНазвание: B\n", "=== ЗАПИСЬ ===");assert(broken.errors.size()==2);
 auto src=read(fs::u8path(root+"/data/Предметы.txt"));auto q=parse(src,"=== ЗАПИСЬ ===");q.blocks[0].fields["Источник"]="Аниме; Сериал";thrown=false;try{makeEntry(q.blocks[0],"Предметы");}catch(...){thrown=true;}assert(thrown);
 std::vector<Entry> big;big.reserve(20000);for(int i=0;i<20000;++i){Entry e=all[i%all.size()];e.id=std::to_string(i);e.rarity=(i%101)/10.;big.push_back(e);}auto start=std::chrono::steady_clock::now();for(int i=0;i<100;++i)p=pool(big,f,2.5,4.3,6.3,false);auto elapsed=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();for(auto i:p.indices)assert(big[i].rarity>=2.5&&big[i].rarity<=6.3);
 auto reversed=big;std::reverse(reversed.begin(),reversed.end());auto r=pool(reversed,f,2.5,4.3,6.3,false);assert(std::abs(p.total-r.total)<1e-7);
 std::vector<Entry> two={all[0],all[0]};two[0].rarity=1;two[1].rarity=2;auto a=pool(two,f,0,1,10,false);assert(std::abs(a.weights[0]/a.total-.8)<1e-12);int counts[2]={0,0};for(int i=0;i<100000;++i)++counts[pick(a,(i+.5)/100000.)];assert(counts[0]==80000&&counts[1]==20000);
 std::cout<<"PASS: parsing, 5 entries, 8 presets, exclusions, combined filters, random bypass, inclusive bounds, invalid input, duplicate fields, one source and fandom, legacy single-fandom field, empty pool, exact probabilities, order independence.\n";std::cout<<"20,000 records: average filter + weights time "<<elapsed/100<<" ms.\n";
}
