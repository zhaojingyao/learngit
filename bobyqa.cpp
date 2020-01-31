//hello
/**
  UT-Dallas
  test.cpp
Purpose: Design Test

@author Zhaori Bi
@version 1.0 11/19/2016
 */

#include <iostream>
#include <string>
#include <regex>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <stdlib.h>
#include <iterator>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include "./dlib/optimization.h"
#include "sqlite3.h"
#include "Auxi.hpp"
#include "ReadPara.hpp"
#include "SendWait.hpp"
#include "ReadSpec.hpp"
#include "cost.hpp"
#include "decry.hpp"

extern "C"{
using namespace std;
using namespace dlib;
//typedef matrix<double,0,1> column_vector;
//#define MaxN  0x3f3f3f3f
//#define MinN  0xc0c0c0c0

//Initialize data
int ID = 0;
int taskID = 0;
std::vector<struct setting> all_settings=readspec();
struct setting FOM_info=readFOM();
std::vector<std::string> paranames = get_names();
std::vector<double> Objective;						
std::vector<double> Spec;							
std::vector< std::vector<double> > Result;

/********************** generate random number *************************/
/*
int rand_(int num){
  return num;
}
*/

const double randNum(const double ub, const double lb){
  double result=0;
  double f= (double)std::rand()/RAND_MAX;
  result=lb+ f*(ub-lb);
  return result;	
}

const double evaluate_wrap(const column_vector& m){
  double res;
  if(send_wait(m)){
      if(updateSetting(taskID)){
        //cout<<"one point completed!"<<endl;
        res = cost_();
        sendcost(res);
        //cout<<"total cost: "<<res<<endl; 
        //cost.push_back(res);
        //cost_taskID.push_back(taskID);       
      }
    }
  cout<<"###########current cost = "<<res<<endl;
  return res;

}

int main(int SPNum){
  cout<<" ************************************************************************"<<endl;
  cout<<" ********************** Welcome to DesignEasy ***************************"<<endl;	
  cout<<" ************************************************************************"<<endl;
  cout<<" *****************************bobyqa*************************************"<<endl;
  if(!decry()) {
    return -1;
  }
  int PC;
  //std::vector<struct setting> all_settings=readspec();
  int Dim = getDim();
  column_vector starting_point(Dim);
  column_vector cur_best(Dim);
  int best_taskID = 0;
  std::vector<double> lower; //<3.45e-6,1.08e-6>
  std::vector<double> upper; //<1.38e-5,4.30e-6>
  std::vector< std::vector<double> > lu;
  double trivalCost=1e9;
  double fbest=1e9;
  int nInterPoint=2*Dim+1;
  srand(time(NULL));
  int MAXITER=50;
  string result;
  //collect parameters bounds
  lu = get_bound();
  lower = lu[0];
  upper = lu[1];


  for(PC=0;PC<SPNum;++PC){
    for(int i=0;i<Dim;++i){
      starting_point(i)=randNum(10,-10);			//	generate random starting Points
      cout<<"starting_point["<<i<<"] = "<<starting_point(i)<<endl;
    }
    find_min_bobyqa(evaluate_wrap, starting_point, 
		    nInterPoint,    					//	number of interpolation points 2*x.size()+1
	            uniform_matrix<double>(Dim,1, -10),  			//	lower bound constraint
		    uniform_matrix<double>(Dim,1, 10),   			//	upper bound constraint
		    5,     							//	initial trust region radius
		    1e-6,  							//	stopping trust region radius
		    MAXITER);
    
    //cout<<"####################trivalCost1 = "<<trivalCost<<endl;
    trivalCost = evaluate_wrap(starting_point);
    //cout<<"####################trivalCost = "<<trivalCost<<endl;
    if(fbest>trivalCost){
      for(int i=0;i<Dim;i++){	
        cur_best(i)=starting_point(i);			//	update the best candidate
        cout<<"starting_point["<<i<<"] = "<<starting_point(i)<<endl;
      }
      fbest=trivalCost;	
      cout<<"!!!!!!!!!!!!!!!!current best cost = "<<fbest<<endl;

      //save best_point
      evaluate_wrap(cur_best);
      best_taskID = taskID;		
    }
  }
  

  cout<<" **********************Optimization Finished !******************************"<<endl;
  cout<<" ************************************************************************"<<endl;
  cout<<" The best taskID is: *******************************************************"<<endl;	
  cout<<best_taskID<<endl;
  
  return best_taskID;
}
}
