//#####################################################################
// Copyright 2004-2007, Geoffrey Irving, Frank Losasso, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class LOG_ENTRY
//##################################################################### 
#ifndef __LOG_ENTRY__
#define __LOG_ENTRY__

#include <Utilities/LOG.h>
#include <stdio.h>
#include <string>
#include <Utilities/TIMER.h>
#include <Utilities/override.h>
#include <iostream>
#include <fstream>

namespace PhysBAM{

class LOG_ENTRY:public NONCOPYABLE
{
public:
    LOG_ENTRY* parent;
    int depth;
    int timer_id;
    double time;
    double timer_start_time;
    std::string name;
    bool end_on_separate_line,log_file_end_on_separate_line;
    static bool start_on_separate_line,log_file_start_on_separate_line;
    static bool needs_indent,log_file_needs_indent;
    int& verbosity_level;

    LOG_ENTRY(LOG_ENTRY* parent_input,const int depth_input,const int timer_id_input,const std::string& name_input,int& verbosity_level_input)
        :parent(parent_input),depth(depth_input),timer_id(timer_id_input),time(0),name(name_input),verbosity_level(verbosity_level_input)
    {
        end_on_separate_line=false;log_file_end_on_separate_line=false;
        timer_start_time=TIMER::Singleton()->Peek_Time(timer_id);
    }

    virtual ~LOG_ENTRY()
    {}

    void Start()
    {if(depth<=verbosity_level){
        if(start_on_separate_line) putchar('\n');start_on_separate_line=needs_indent=true;
        printf("%*s%-*s",2*depth,"",50-2*depth,name.c_str());fflush(stdout);}
        LOG::LOG_CLASS* instance=LOG::Instance();
    if(instance->log_file){
        if(log_file_start_on_separate_line) putc('\n',instance->log_file);log_file_start_on_separate_line=log_file_needs_indent=true;
        if(instance->xml) Start_XML();
        else fprintf(instance->log_file,"%*s%-*s",2*depth,"",50-2*depth,name.c_str());
        fflush(instance->log_file);}
    timer_start_time=TIMER::Singleton()->Peek_Time(timer_id);}

    virtual void Start_XML()
    {fprintf(LOG::Instance()->log_file,"%*s<scope name=\"%s\">",2*depth,"",name.c_str());}

    void Stop()
    {double time_since_start=(TIMER::Singleton()->Peek_Time(timer_id)-timer_start_time)/1000;
    if(depth<=verbosity_level){
        if(end_on_separate_line){
            if(start_on_separate_line) putchar('\n');
            printf("%*sEND %-*s",2*depth,"",50-2*depth-4,name.c_str());}
        end_on_separate_line=false;start_on_separate_line=needs_indent=true;
        printf("%8.4f s",time_since_start);fflush(stdout);}
    LOG::LOG_CLASS* instance=LOG::Instance();
    if(instance->log_file){
        if(instance->xml){
            if(log_file_end_on_separate_line){
                if(log_file_start_on_separate_line) putc('\n',instance->log_file);
                fprintf(instance->log_file,"%*s",2*depth,"");}
            log_file_end_on_separate_line=false;log_file_start_on_separate_line=log_file_needs_indent=true;
            fprintf(instance->log_file,"<time value=\"%f\"/></scope>",time_since_start);fflush(instance->log_file);}
        else{
            if(log_file_end_on_separate_line){
                if(log_file_start_on_separate_line) putc('\n',instance->log_file);
                fprintf(instance->log_file,"%*sEND %-*s",2*depth,"",50-2*depth-4,name.c_str());}
            log_file_end_on_separate_line=false;log_file_start_on_separate_line=log_file_needs_indent=true;
            fprintf(instance->log_file,"%8.4f s",time_since_start);fflush(instance->log_file);}}
    time+=time_since_start;}

    virtual LOG_ENTRY* Get_Stop_Time()
    {Stop();return parent;}

    virtual LOG_ENTRY* Get_New_Scope(const std::string& new_scope_identifier,const std::string& new_name)
    {Stop();return parent->Get_New_Scope(new_scope_identifier,new_name);}

    virtual LOG_ENTRY* Get_New_Item(const std::string& new_name)
    {Stop();return parent->Get_New_Item(new_name);}

    virtual LOG_ENTRY* Get_Pop_Scope()
    {Stop();return parent->Get_Pop_Scope();}

    virtual void Dump_Log(FILE* output)
    {fprintf(output,"%*s%-*s%8.4f s\n",2*depth,"",50-2*depth,name.c_str(),time);fflush(output);}

    virtual void Dump_Names(FILE* output)
    {fprintf(output,"%*s%-*s",2*depth,"",50-2*depth,name.c_str());fflush(output);}

//##################################################################### 
};
}
#endif
