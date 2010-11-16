//#####################################################################
// Copyright 2004-2008, Geoffrey Irving, Frank Losasso, Andrew Selle, Jonathan Su.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#include <Utilities/LOG.h>
#include <Utilities/LOG_ENTRY.h>
#include <Utilities/LOG_SCOPE.h>
#include <Utilities/TIMER.h>
#include <Data_Structures/ELEMENT_ID.h>
#include <Geometry/BOX.h>
#include <Read_Write/FILE_UTILITIES.h>
#include <sstream>
namespace PhysBAM{
namespace LOG{
//#####################################################################
namespace{
scoped_ptr<LOG_CLASS> private_instance;
scoped_ptr<std::streambuf> cout_buffer;
scoped_ptr<std::streambuf> cerr_buffer;
bool cout_initialized=false;
bool cerr_initialized=false;

template<class T>
struct INITIALIZATION_HELPER
{
    scoped_ptr<T> object;
    bool& initialized;

    INITIALIZATION_HELPER(T* object,bool& initialized)
        :object(object),initialized(initialized)
    {
        initialized=true;
    }

    ~INITIALIZATION_HELPER()
    {initialized=false;}
};

void Dump_Log_Helper(LOG_CLASS& instance,const bool suppress_cout);
}
//static int verbosity_level=(1<<30)-1;
//static bool log_file_temporary=false;
//static bool xml=true;
//#####################################################################
// Class LOG_COUT_BUFFER
//#####################################################################
class LOG_COUT_BUFFER:public std::stringbuf
{
    int sync()
    {if(!Instance()->suppress_cout && Instance()->current_entry->depth<Instance()->verbosity_level){
        if(LOG_ENTRY::start_on_separate_line) putchar('\n');
        std::string buffer=str();
        for(size_t start=0;start<buffer.length();){
            size_t end=buffer.find('\n',start);
            if(LOG_ENTRY::needs_indent){printf("%*s",2*Instance()->current_entry->depth+2,"");LOG_ENTRY::needs_indent=false;}
            fputs(buffer.substr(start,end-start).c_str(),stdout);
            if(end!=std::string::npos){putchar('\n');LOG_ENTRY::needs_indent=true;start=end+1;}
            else break;}
        LOG_ENTRY::start_on_separate_line=false;Instance()->current_entry->end_on_separate_line=true;
        fflush(stdout);}
    if(Instance()->log_file){
        if(LOG_ENTRY::log_file_start_on_separate_line) putc('\n',Instance()->log_file);
        std::string buffer=str();
        for(size_t start=0;start<buffer.length();){
            size_t end=buffer.find('\n',start);
            if(LOG_ENTRY::log_file_needs_indent){
                fprintf(Instance()->log_file,"%*s",2*Instance()->current_entry->depth+2,"");LOG_ENTRY::log_file_needs_indent=false;
                if(Instance()->xml) fputs("<print>",Instance()->log_file);}
            fputs(buffer.substr(start,end-start).c_str(),Instance()->log_file);
            if(end!=std::string::npos){
                if(Instance()->xml) fputs("</print>",Instance()->log_file);
                putc('\n',Instance()->log_file);LOG_ENTRY::log_file_needs_indent=true;start=end+1;}
            else break;}
        LOG_ENTRY::log_file_start_on_separate_line=false;Instance()->current_entry->log_file_end_on_separate_line=true;
        fflush(Instance()->log_file);}
    str("");return std::stringbuf::sync();}
};
//#####################################################################
// Class LOG_CERR_BUFFER
//#####################################################################
class LOG_CERR_BUFFER:public std::stringbuf
{
    int sync()
    {if(!Instance()->suppress_cerr){
        if(LOG_ENTRY::start_on_separate_line) putchar('\n');LOG_ENTRY::start_on_separate_line=false;
        fputs(str().c_str(),stderr);}
    if(Instance()->log_file){
        if(LOG_ENTRY::log_file_start_on_separate_line) putc('\n',Instance()->log_file);LOG_ENTRY::log_file_start_on_separate_line=false;
        std::string buffer=str();
        for(size_t start=0;start<buffer.length();){
            size_t end=buffer.find('\n',start);
            if(Instance()->xml) fputs("<error>",Instance()->log_file);
            fputs(buffer.substr(start,end-start).c_str(),Instance()->log_file);
            if(Instance()->xml) fputs("</error>",Instance()->log_file);
            putc('\n',Instance()->log_file);
            if(end!=std::string::npos) start=end+1;
            else break;}}
    str("");return std::stringbuf::sync();}
};
//#####################################################################
// Constructor
//#####################################################################
LOG_CLASS::LOG_CLASS(const bool suppress_cout_input,const bool suppress_cerr_input,const bool suppress_timing_input,const int verbosity_level_input,const bool cache_initial_output)
    :timer_singleton(TIMER::Singleton()),suppress_cout(suppress_cout_input),suppress_cerr(suppress_cerr_input),suppress_timing(suppress_timing_input),log_file(0),verbosity_level(verbosity_level_input),xml(true)
{
    private_instance.reset(this);
    if(cache_initial_output){
        log_file=FILE_UTILITIES::Temporary_File();
        if(!log_file) PHYSBAM_FATAL_ERROR(str(boost::format("Couldn't create temporary log file tmpfile=%s")%log_file));
        log_file_temporary=true;}
    timer_id=timer_singleton->Register_Timer();
    cout_buffer.reset(new LOG_COUT_BUFFER);cout.rdbuf(cout_buffer.get());
    cerr_buffer.reset(new LOG_CERR_BUFFER);cerr.rdbuf(cerr_buffer.get());
    root=new LOG_SCOPE(0,0,timer_id,"SIMULATION","Simulation",verbosity_level);
    current_entry=root;root->Start();
}
//#####################################################################
// Destructor
//#####################################################################
LOG_CLASS::~LOG_CLASS()
{
    while(current_entry!=0) current_entry=current_entry->Get_Pop_Scope();
    Dump_Log_Helper(*this,suppress_cout);
    if(log_file) fclose(log_file);
    if(cerr_initialized) cout.rdbuf(std::cout.rdbuf());
    if(cerr_initialized) cerr.rdbuf(std::cerr.rdbuf());
    timer_singleton->Release_Timer(timer_id);
    delete root;
}
//#####################################################################
// Function cout_Helper
//#####################################################################
std::ostream&
cout_Helper()
{
    static INITIALIZATION_HELPER<std::ostream> helper(new std::ostream(std::cout.rdbuf()),cout_initialized); // Necessary for DLLs to work. Cannot use static class data across dlls
    return *helper.object;
}
//#####################################################################
// Function cerr_Helper
//#####################################################################
std::ostream&
cerr_Helper()
{
    static INITIALIZATION_HELPER<std::ostream> helper(new std::ostream(std::cerr.rdbuf()),cerr_initialized); // Necessary for DLLs to work. Cannot use static class data across dlls
    return *helper.object;
}
//#####################################################################
// Function Instance
//#####################################################################
LOG_CLASS* Instance()
{
    if(!private_instance) Initialize_Logging();
    return private_instance.get();
}
//#####################################################################
// Function Initialize_Logging
//#####################################################################
void Initialize_Logging(const bool suppress_cout_input,const bool suppress_timing_input,const int verbosity_level_input,const bool cache_initial_output)
{
    new LOG_CLASS(suppress_cout_input,false,suppress_timing_input,verbosity_level_input-1,cache_initial_output);
}
//#####################################################################
// Function Copy_Log_To_File
//#####################################################################
void LOG_CLASS::
Copy_Log_To_File(const std::string& filename,const bool append)
{
    FILE* temporary_file=0;
    if(log_file && log_file_temporary){
        temporary_file=log_file;log_file=0;}
    if(log_file){
        if(LOG_ENTRY::log_file_start_on_separate_line) putc('\n',log_file);
        Instance()->root->Dump_Log(log_file);
        fclose(log_file);log_file=0;}
    if(!filename.empty()){
        if(append){
            log_file=fopen(filename.c_str(),"a");
            if(!log_file) PHYSBAM_FATAL_ERROR(str(boost::format("Can't open log file %s for append")%filename));
            putc('\n',log_file);}
        else{
            log_file=fopen(filename.c_str(),"w");
            if(!log_file) PHYSBAM_FATAL_ERROR(str(boost::format("Can't open log file %s for writing")%filename));}
        if(!temporary_file){
            Instance()->root->Dump_Names(log_file);
            LOG_ENTRY::log_file_start_on_separate_line=LOG_ENTRY::log_file_needs_indent=Instance()->current_entry->log_file_end_on_separate_line=true;}
        else{
            fflush(temporary_file);fseek(temporary_file,0,SEEK_SET);
            ARRAY<char> buffer(4096,false);
            for(;;){
                int n=(int)fread(buffer.Get_Array_Pointer(),sizeof(char),buffer.m,temporary_file);
                fwrite(buffer.Get_Array_Pointer(),sizeof(char),n,log_file);
                if(n<buffer.m) break;}
            fflush(log_file);}}
    if(temporary_file) fclose(temporary_file);
    log_file_temporary=false;
}
//#####################################################################
// Function Finish_Logging
//#####################################################################
void
Finish_Logging()
{
    private_instance.reset();
}
//#####################################################################
// Function Time_Helper
//#####################################################################
void LOG_CLASS::
Time_Helper(const std::string& label)
{
    LOG_CLASS* instance=Instance();
    instance->current_entry=instance->current_entry->Get_New_Item(label);
    instance->current_entry->Start();
}
//#####################################################################
// Function Stop_Time
//#####################################################################
void
Stop_Time()
{
    LOG_CLASS* instance=Instance();
    if(instance->suppress_timing) return;
    instance->current_entry=instance->current_entry->Get_Stop_Time();
}
//#####################################################################
// Function Stat
//#####################################################################
template<class T_VALUE> void
Stat(const std::string& label,const T_VALUE& value)
{
    std::stringstream s;s<<value;
    LOG_CLASS* instance=Instance();
    if(instance->suppress_timing) return;
    if(instance->current_entry->depth<instance->verbosity_level){
        if(LOG_ENTRY::start_on_separate_line) putchar('\n');
        if(LOG_ENTRY::needs_indent) printf("%*s",2*instance->current_entry->depth+2,"");
        printf("%s = %s\n",label.c_str(),s.str().c_str());
        LOG_ENTRY::start_on_separate_line=false;LOG_ENTRY::needs_indent=instance->current_entry->end_on_separate_line=true;}
    if(instance->log_file){
        if(LOG_ENTRY::log_file_start_on_separate_line) putc('\n',instance->log_file);
        if(LOG_ENTRY::log_file_needs_indent) fprintf(instance->log_file,"%*s",2*instance->current_entry->depth+2,"");
        if(instance->xml) fprintf(instance->log_file,"<stat name=\"%s\" value=\"%s\"/>\n",label.c_str(),s.str().c_str());
        else fprintf(instance->log_file,"%s = %s\n",label.c_str(),s.str().c_str());
        LOG_ENTRY::log_file_start_on_separate_line=false;LOG_ENTRY::log_file_needs_indent=instance->current_entry->log_file_end_on_separate_line=true;}
}
//#####################################################################
// Function Push_Scope
//#####################################################################
void LOG_CLASS::
Push_Scope(const std::string& scope_identifier,const std::string& scope_name)
{
    LOG_CLASS* instance=Instance();
    if(instance->suppress_timing) return;
    instance->current_entry=instance->current_entry->Get_New_Scope(scope_identifier,scope_name);
    instance->current_entry->Start();
}
//#####################################################################
// Function Pop_Scope
//#####################################################################
void LOG_CLASS::
Pop_Scope()
{
    LOG_CLASS* instance=Instance();
    if(instance->suppress_timing) return;
    instance->current_entry=instance->current_entry->Get_Pop_Scope();
    if(instance->current_entry==0){LOG::cerr<<"Could not pop scope. Defaulting to root"<<std::endl;instance->current_entry=instance->root;}
}
//#####################################################################
// Function Reset
//#####################################################################
void
Reset()
{
    LOG_CLASS* instance=Instance();
    if(instance->suppress_timing) return;
    delete instance->root;
    instance->root=new LOG_SCOPE(0,0,instance->timer_id,"SIMULATION","Simulation",instance->verbosity_level);instance->current_entry=instance->root;
}
//#####################################################################
// Function Dump_Log
//#####################################################################
namespace{
void
Dump_Log_Helper(LOG_CLASS& instance,const bool suppress_cout)
{
    if(instance.suppress_timing) return;
    if(LOG_ENTRY::start_on_separate_line){putchar('\n');LOG_ENTRY::start_on_separate_line=false;}
    if(!suppress_cout) instance.root->Dump_Log(stdout);
    if(instance.log_file){
        if(LOG_ENTRY::log_file_start_on_separate_line){putc('\n',instance.log_file);LOG_ENTRY::log_file_start_on_separate_line=false;}
        if(!instance.xml) instance.root->Dump_Log(instance.log_file);}
}
}
void
Dump_Log()
{
    LOG_CLASS& instance=*Instance();
    Dump_Log_Helper(instance,instance.suppress_cout);
}
//#####################################################################
template void Stat(const std::string&,const int&);
template void Stat(const std::string&,const bool&);
template void Stat(const std::string&,const float&);
template void Stat(const std::string&,const double&);
template void Stat(const std::string&,const BOX<VECTOR<int,1> >&);
template void Stat(const std::string&,const BOX<VECTOR<float,1> >&);
template void Stat(const std::string&,const BOX<VECTOR<double,1> >&);
#ifndef COMPILE_ID_TYPES_AS_INT
template void Stat(const std::string&,const FRAGMENT_ID&);
#endif
}
}
