/**
 *  @copyright Copyright 2016 The J-PET Framework Authors. All rights reserved.
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may find a copy of the License in the LICENCE file.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *  @file Calib1.cpp
 */

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <cctype>
#include <JPetWriter/JPetWriter.h>
#include "Calib1.h"
#include "TF1.h"
#include "TString.h"
#include <TDirectory.h>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

Calib1::Calib1(const char * name, const char * description):JPetTask(name, description){}

void Calib1::init(const JPetTaskInterface::Options& opts){

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	fBarrelMap.buildMappings(getParamBank());
// create histograms for time differences at each slot and each threshold

	for(auto & scin : getParamBank().getScintillators()){//over slots

		for (int thr=1;thr<=4;thr++){//over threshold

//histos for leading edge
			const char * histo_name_l = formatUniqueSlotDescription(scin.second->getBarrelSlot(), thr, "timeDiffAB_leading_");
			getStatistics().createHistogram( new TH1F(histo_name_l, histo_name_l, 100, -20., 20.) );
//histograms for leading edge refference detector time difference
                        const char * histo_name_Ref_l = formatUniqueSlotDescription(scin.second->getBarrelSlot(), thr, "timeDiffRef_leading_");
			getStatistics().createHistogram( new TH1F(histo_name_Ref_l, histo_name_Ref_l, 100, -2000., 2000.) );
			
//histos for trailing edge			
			const char * histo_name_t = formatUniqueSlotDescription(scin.second->getBarrelSlot(), thr, "timeDiffAB_trailing_");
			getStatistics().createHistogram( new TH1F(histo_name_t, histo_name_t, 100, -20., 20.) );
//histograms for leading edge refference detector time difference
                        const char * histo_name_Ref_t = formatUniqueSlotDescription(scin.second->getBarrelSlot(), thr, "timeDiffRef_trailing_");
			getStatistics().createHistogram( new TH1F(histo_name_Ref_t, histo_name_Ref_t, 100, -2000., 2000.) );
		}
	}


// create histograms for time diffrerence vs slot ID

	for(auto & layer : getParamBank().getLayers()){

		for (int thr=1;thr<=4;thr++){

//histos for leading edge
			const char * histo_name_l = Form("TimeDiffVsID_layer%d_thr%d_leading", fBarrelMap.getLayerNumber(*layer.second), thr);
			const char * histo_titile_l = Form("%s;Slot ID; TimeDiffAB [ns]", histo_name_l);
//histograms for leading edge refference detector time difference
//			const char * histo_name_Ref_l = Form("TimeDiffReffVsID_layer%d_thr%d_leading", fBarrelMap.getLayerNumber(*layer.second), thr);
//			const char * histo_titile_Ref_l = Form("%s;Slot ID; TimeDiffRef_L [ns]", histo_name_Ref_l);

//histos for trailing edge
			const char * histo_name_t = Form("TimeDiffVsID_layer%d_thr%d_trailing", fBarrelMap.getLayerNumber(*layer.second), thr);
			const char * histo_titile_t = Form("%s;Slot ID; TimeDiffAB [ns]", histo_name_t);
			//histograms for leading edge refference detector time difference
			//			const char * histo_name_Ref_t = Form("TimeDiffReffVsID_layer%d_thr%d_trailing", fBarrelMap.getLayerNumber(*layer.second), thr);
			//			const char * histo_titile_Ref_t = Form("%s;Slot ID; TimeDiffRef_T [ns]", histo_name_Ref_t);
			

			int n_slots_in_layer = fBarrelMap.getNumberOfSlots(*layer.second);

			getStatistics().createHistogram( new TH2F(histo_name_l, histo_titile_l, n_slots_in_layer, 0.5, n_slots_in_layer+0.5,
								  120, -20., 20.) );

			getStatistics().createHistogram( new TH2F(histo_name_t, histo_titile_t, n_slots_in_layer, 0.5, n_slots_in_layer+0.5,
								  120, -20., 20.) );
			//			getStatistics().createHistogram( new TH2F(histo_name_Ref_l, histo_titile_Ref_l, n_slots_in_layer, 0.5, n_slots_in_layer+0.5,
			//								  120, -20., 20.) );
			//			getStatistics().createHistogram( new TH2F(histo_name_Ref_t, histo_titile_Ref_t, n_slots_in_layer, 0.5, n_slots_in_layer+0.5,
			//					  120, -20., 20.) );
		}
	}


}

//TimeDiffVsID_layer%d_thr%d_leading
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Calib1::exec(){
  double RefTimeLead[4]={-1.e43,-1.e43,-1.e43,-1.e43};
  double RefTimeTrail[4]={-1.e43,-1.e43,-1.e43,-1.e43};
	//getting the data from event in propriate format
	if(auto hit =dynamic_cast<const JPetHit*const>(getEvent())){
     	  //
	  //taking refference detector hits times (scin=193)
	  //
	  int PMid = hit->getSignalB().getRecoSignal().getRawSignal().getPM().getID();
	  //	  int TimeWin = hit->getSignalB().getRecoSignal().getRawSignal().getTimeWindowIndex();
	  //std::cout << "PMID= "<< PMid << std::endl;
	  if(PMid==385){
	    auto lead_times_B = hit->getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
	    auto trail_times_B = hit->getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Trailing);                                                   
	    for(auto & thr_time_pair : lead_times_B){
	      int thr = thr_time_pair.first;
	      RefTimeLead[thr] = thr_time_pair.second;
	      // std::cout <<"RefTimeLead=" <<RefTimeLead[thr] <<std::endl;
	    }
	    for(auto & thr_time_pair : trail_times_B){
	      int thr = thr_time_pair.first;
	      RefTimeTrail[thr] = thr_time_pair.second;
	      // std::cout<<"RefTimeTrail= " <<RefTimeTrail[thr] <<std::endl;
	    }
	    RefTimesL.push_back(RefTimeLead[1]);
	    RefTimesT.push_back(RefTimeTrail[1]);
	  }
	  //
	  if (hitsCalib.empty()) {
	       hitsCalib.push_back(*hit);  
	  } else {
	    if (hitsCalib[0].getTimeWindowIndex() == hit->getTimeWindowIndex()) {
		hitsCalib.push_back(*hit);
	    } else {
	      for (auto i = hitsCalib.begin(); i != hitsCalib.end(); i++) {
		fillHistosForHit(*i,RefTimesL,RefTimesT);
              }	
	        hitsCalib.clear();
		RefTimesL.clear();
		RefTimesT.clear();
	        hitsCalib.push_back(*hit);
		//If the first hit in the next time window is the refference detector hit save its times
		if(PMid==385){
		  auto lead_times_B = hit->getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
		  auto trail_times_B = hit->getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Trailing);
		  for(auto & thr_time_pair : lead_times_B){
		    int thr = thr_time_pair.first;
		    RefTimeLead[thr] = thr_time_pair.second;
		  }
		  for(auto & thr_time_pair : trail_times_B){
		    int thr = thr_time_pair.first;
		    RefTimeTrail[thr] = thr_time_pair.second;
		  }
		  RefTimesL.push_back(RefTimeLead[1]);
		  RefTimesT.push_back(RefTimeTrail[1]);
		}
	    }
	  }



	  
      	  if(PMid==385){
	      hitsCalib.push_back(*hit);
	  fWriter->write(*hit);
	}	
	/*	  if(hitsCalib.size()>1) {
	   std::cout <<"hitsCalib.size()= " << hitsCalib.size() << std::endl;
	  }
	 for (auto i = hitsCalib.begin(); i != hitsCalib.end(); i++) {            
     	     fillHistosForHit(*i,RefTimesL,RefTimesT);
	 
	 //   hitsCalib.clear();
	 //   RefTimesL.clear();
	 //   RefTimesT.clear();
	 */
	}  
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Calib1::terminate(){

//getting Layer and Slots numbers from analysing files name

//gDirectory->GetFile()->GetName(); //path and name of output file example: 
//zwraca np. "/home/mskurzok/j-pet-framework-examples/build/data_ref/Layer1_Slot16.phys.hit.means.root"
//
std::vector<int> vectorOfNumbers;
std::string str(gDirectory->GetFile()->GetName());//getting directory of analysing file

std::cout <<"Calib1::terminate : File with time difference histograms to fit is " << str << std::endl;

int number=0;

for (unsigned int i=0; i < str.size(); i++){

	if (isdigit(str[i])){

          std::stringstream ss;
          ss<<str[i];
          ss>>number; //convert string into int and store it in "asInt"

	  std::cout << "number" << number << std::endl;

         vectorOfNumbers.push_back(number);

	}
}
/*
 //int a = vectorOfNumbers[0];
 //int b = vectorOfNumbers[1];


//////////////////////////////////////////////////////////////////////////////////////////////////////

// save timeDiffAB mean values for each slot and each threshold in a JPetAuxilliaryData object
	// so that they are available to the consecutive modules
	getAuxilliaryData().createMap("timeDiffAB mean values");


//create output txt file with calibration parameters 
 
	std::ofstream results_fit;
        results_fit.open("results.txt", std::ios::app); //file will be overwritten

	for(auto & slot : getParamBank().getBarrelSlots()){

		for (int thr=1;thr<=4;thr++){

			const char * histo_name_l = formatUniqueSlotDescription(*(slot.second), thr, "timeDiffAB_leading_");
			double mean_l = getStatistics().getHisto1D(histo_name_l).GetMean();
			getAuxilliaryData().setValue("timeDiffAB mean values", histo_name_l, mean_l);

			TH1F* histoToSave_leading = &(getStatistics().getHisto1D(histo_name_l));

			const char * histo_name_t = formatUniqueSlotDescription(*(slot.second), thr, "timeDiffAB_trailing_");
			double mean_t = getStatistics().getHisto1D(histo_name_t).GetMean();
			getAuxilliaryData().setValue("timeDiffAB mean values", histo_name_t, mean_t);

			TH1F* histoToSave_trailing = &(getStatistics().getHisto1D(histo_name_t));

//non zero histos 
// slot.first - ID
// slot.second - wskaznik na JPetBarrelSlot
//save fit parameters only for layerX and SlotY
//fit just for proper slot

			if(histoToSave_leading->GetEntries() != 0 && histoToSave_trailing->GetEntries() != 0
                          && (slot.second)->getLayer().getID()== vectorOfNumbers[0]
                          && (slot.first==(10*vectorOfNumbers[1]+vectorOfNumbers[2]))){

			//if(histoToSave->GetEntries() != 0 && (slot.second)->getLayer().getID()== 1 && (slot.first==20)){

			int highestBin_l = histoToSave_leading->GetBinCenter(histoToSave_leading->GetMaximumBin());
			histoToSave_leading->Fit("gaus","","", highestBin_l-5, highestBin_l+5);
			TCanvas* cl = new TCanvas();
			histoToSave_leading->Draw();
			std::string sHistoName_l = histo_name_l; 
			sHistoName_l+=".png";


			int highestBin_t = histoToSave_trailing->GetBinCenter(histoToSave_trailing->GetMaximumBin());
			histoToSave_trailing->Fit("gaus","","", highestBin_t-5, highestBin_t+5);
			TCanvas* ct = new TCanvas();
			histoToSave_trailing->Draw();
			std::string sHistoName_t = histo_name_t; 
			sHistoName_t+=".png";


			//c->SaveAs( sHistoName.c_str());
		
			TF1 *fit_l = histoToSave_leading->GetFunction("gaus");
			TF1 *fit_t = histoToSave_trailing->GetFunction("gaus");

			double position_peak_l = fit_l->GetParameter(1);
   			double position_peak_error_l=fit_l->GetParError(1);
			double sigma_peak_l =fit_l->GetParameter(2);
			double chi2_ndf_l = fit_l->GetChisquare()/fit_l->GetNDF();

			double position_peak_t = fit_t->GetParameter(1);
   			double position_peak_error_t=fit_t->GetParError(1);
			double sigma_peak_t =fit_t->GetParameter(2);
			double chi2_ndf_t = fit_t->GetChisquare()/fit_t->GetNDF();

//save parameters in .txt file: Layer, Slot, Position leding, Error_position leading,
//Width leading, Chi2/ndf leading, Position trailing, Error_position trailing, Width trailing,
//Chi2/ndf trailing

		results_fit <<  (slot.second)->getLayer().getID() << "\t" <<  slot.first << "\t" << thr << "\t" << position_peak_l << "\t"
                << position_peak_error_l  << "\t" << sigma_peak_l  << "\t" << chi2_ndf_l  << "\t" << position_peak_t << "\t" << position_peak_error_t
                << "\t" << sigma_peak_t  << "\t" << chi2_ndf_t  << "\t" << std::endl;
		std::cout <<  (slot.second)->getLayer().getID() << "\t" <<  slot.first << "\t" << thr << "\t" << position_peak_l << "\t" << 
                position_peak_error_l  << "\t" << sigma_peak_l  << "\t" << chi2_ndf_l  << "\t" << position_peak_t << "\t" << position_peak_error_t
                << "\t" << sigma_peak_t  << "\t" << chi2_ndf_t  << "\t" << std::endl;
			}
			
		}
	}
	results_fit.close();
*/
}

//const char * histo_name = Form("TimeDiff_layer_%d_slot_%d_thr_%d", LayerNR, SlotNR, thr);
//getStatistics().createHistogram( new TH1F(histo_name, histo_name, 2000, -20., 20.));

//////////////////////////////////

void Calib1::fillHistosForHit(const JPetHit & hit, const std::vector<double>  & RefTimesL, const std::vector<double> & RefTimesT){

	auto lead_times_A = hit.getSignalA().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
	auto trail_times_A = hit.getSignalA().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Trailing);

	auto lead_times_B = hit.getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
        auto trail_times_B = hit.getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Trailing);
	//
	//        for(int i=0; i<RefTimesL.size();i++){
	//	  std::cout <<"RefTimesL" << RefTimesL[i] << std::endl;
	//	  std::cout <<"RefTimesT" << RefTimesT[i] << std::endl;
	//        }

//leading edge
//	std::cout <<"RefTimesL.size()= " << RefTimesL.size() << std::endl;
	for(auto & thr_time_pair : lead_times_A){

		int thr = thr_time_pair.first;

		if( lead_times_B.count(thr) > 0 ){ // if there was leading time at the same threshold at opposite side

			double timeDiffAB_l = lead_times_A[thr] - lead_times_B[thr];
			timeDiffAB_l/= 1000.; // we want the plots in ns instead of ps

			// fill the appropriate histogram
			const char * histo_name_l = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffAB_leading_");
			getStatistics().getHisto1D(histo_name_l).Fill( timeDiffAB_l);

			// fill the timeDiffAB vs slot ID histogram

			int layer_number = fBarrelMap.getLayerNumber( hit.getBarrelSlot().getLayer() );
			int slot_number = fBarrelMap.getSlotNumber( hit.getBarrelSlot() );
			getStatistics().getHisto2D(Form("TimeDiffVsID_layer%d_thr%d_leading", layer_number, thr)).Fill( slot_number,
															timeDiffAB_l);
//Assuming that the refference signal is always the first one we take the first hit after the hit in the refference detector
//Here we take advantage of the fact that the hits are somehow ordered in time, which is true for now if this is not true we
//have to insert this part of the code (and the one on the beginning of this function outside this function and create a vecor
//of hits and then for each refference hit look for the next hit in time
			for(int i = 0; i < RefTimesL.size();i++) {
			double timeDiffHit_L = (lead_times_A[thr] + lead_times_B[thr])/2. -RefTimesL[i];
			timeDiffHit_L = timeDiffHit_L/1000000.;//ps -> micros
			const char * histo_name_Ref_l = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffRef_leading_");
			getStatistics().getHisto1D(histo_name_Ref_l).Fill(timeDiffHit_L);
			//			std::cout << "Led "<<i <<" " <<timeDiffHit_L <<" "<< timeDiffAB_l<<" "<<lead_times_A[thr]<<" "<<lead_times_B[thr]<<" "<<RefTimesL[i] <<std::endl;
			}
		}
	}


//trailing
	for(auto & thr_time_pair : trail_times_A){
		int thr = thr_time_pair.first;

		if( trail_times_B.count(thr) > 0 ){ // if there was trailing time at the same threshold at opposite side

			double timeDiffAB_t = trail_times_A[thr] - trail_times_B[thr];
			timeDiffAB_t/= 1000.; // we want the plots in ns instead of ps

			// fill the appropriate histogram
			const char * histo_name_t = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffAB_trailing_");
			getStatistics().getHisto1D(histo_name_t).Fill( timeDiffAB_t);

			// fill the timeDiffAB vs slot ID histogram
			int layer_number = fBarrelMap.getLayerNumber( hit.getBarrelSlot().getLayer() );
			int slot_number = fBarrelMap.getSlotNumber( hit.getBarrelSlot() );
			getStatistics().getHisto2D(Form("TimeDiffVsID_layer%d_thr%d_trailing", layer_number, thr)).Fill( slot_number,
															 timeDiffAB_t);
//Assuming that the refference signal is always the first one we take the first hit after the hit in the refference detector                                                                              
//Here we take advantage of the fact that the hits are somehow ordered in time, which is true for now if this is not true we
//have to insert this part of the code (and the one on the beginning of this function outside this function and create a vecor
//of hits and then for each refference hit look for the next hit in time. 
			for(int i = 0; i < RefTimesT.size();i++) {
                          double timeDiffHit_T = (trail_times_A[thr] + trail_times_B[thr])/2. -RefTimesT[i];
			  timeDiffHit_T = timeDiffHit_T/1000000.; //ps->micros
			  const char * histo_name_Ref_t = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffRef_trailing_");
			  getStatistics().getHisto1D(histo_name_Ref_t).Fill(timeDiffHit_T);
			  //std::cout << "Trail " << i <<" "<<timeDiffHit_T <<" "<<timeDiffAB_t <<" "<<trail_times_A[thr]<<" "<<trail_times_B[thr] <<" "<<RefTimesT[i] <<std::endl;
			  }	
	        }
	}
}


const char * Calib1::formatUniqueSlotDescription(const JPetBarrelSlot & slot, int threshold, const char * prefix = ""){

	int slot_number = fBarrelMap.getSlotNumber(slot);
	int layer_number = fBarrelMap.getLayerNumber(slot.getLayer()); 

	return Form("%slayer_%d_slot_%d_thr_%d",prefix,layer_number,slot_number,threshold);

}
void Calib1::setWriter(JPetWriter* writer){fWriter =writer;}
/*
void Calib1::GetRefHitTimes(const JPetHit & hit, double RefTimeLead[4], double RefTimeTrail[4])
{
  auto lead_times_B = hit.getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
  auto trail_times_B = hit.getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Trailing);                                                                            
                                                   
  for(auto & thr_time_pair : lead_times_B){
    int thr = thr_time_pair.first;
    RefTimeLead[thr] = thr_time_pair.second;
    // std::cout <<"RefTimeLead=" <<RefTimeLead[thr] <<std::endl;
  }
  for(auto & thr_time_pair : trail_times_B){
    int thr = thr_time_pair.first;
    RefTimeTrail[thr] = thr_time_pair.second;
    // std::cout<<"RefTimeTrail= " <<RefTimeTrail[thr] <<std::endl;
  }
}
*/
