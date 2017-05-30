#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include<iostream>
#include <sstream>
#include <regex>
#include <algorithm>
#include <string>
using namespace std;
using namespace cv;
#include <stdio.h>
#include <stdlib.h>

typedef struct RIB{

    string IB;
    string name;
    float IB_confidence;
    float name_confidence;
} ;


string correct_IBAN(string &ll){
  int found;
  string temp="";
  int j=0;
  string strCheck="FR0123456789 ";
  for (unsigned i=0; i<ll.length(); ++i)
  {
    
    found = std::find(strCheck.begin(), strCheck.end(), ll.at(i)) != strCheck.end();
    if(found !=0){
        if(ll.at(i)!=' ' or (i>1 and ll.at(i-1)!=' ') )
            temp=temp+ll.at(i);
    }

  }
  // trim leading spaces
    size_t startpos = temp.find_first_not_of(" \t");
    if( string::npos != startpos )
    {
        temp = temp.substr( startpos );
    }

  if(temp[1]!='R')
  {
     temp[1]='R';

  } 
    
  if(temp[0]!='F')
    temp[0]='F';
  return temp;
}

void getIBAN_Name(tesseract::TessBaseAPI *api, Boxa* box, RIB &rb){
    RIB r;
    r.IB="";
    r.IB_confidence=0;
    r.name="";
    r.name_confidence=0;
    for (int z = 0; z < box->n; z++) {

        BOX* b = boxaGetBox(box, z, L_CLONE);
        api->SetRectangle(b->x, b->y, b->w, b->h);
        char* text = api->GetUTF8Text();
        float confidence = api->MeanTextConf();
        string ll(text);
        std::smatch m;
        //std::regex e1 ("(.*)(FR)(.*)([0-9]{2})(.+)([0-9]{4})(.+)([0-9]{4})(.*)");
        std::regex e1 ("(FR)([0-9]{2})(.*)([0-9]{4})(.*)|(F)(.+)([0-9]{2})(.*)([0-9]{4})|(R)([0-9]{2})(.*)([0-9]{4})(.*)");
        
        std::regex e2 ("(MADEMOISELLE)|(MMME)|(MME)|(MR )|(MLLE)|(M\\.)|(M )|(MONSIEUR )|(MADAME )|(ASS )");
        if(std::regex_search (ll,m,e1))
        {
            r.IB=ll.substr(m.position(0),ll.length());
            //cout<<"found IBAN"<<endl;
            r.IB_confidence=confidence;
        }
        if(std::regex_search (ll,m,e2))
        {
            r.name=ll.substr(m.position(0),ll.length());
            r.name_confidence=confidence;
        }

    }
    rb=r;

}

Mat rotate(Mat src, double angle)
{
    Mat dst;
    Point2f pt(src.cols/2., src.rows/2.);    
    Mat r = getRotationMatrix2D(pt, angle, 1.0);
    warpAffine(src, dst, r, Size(src.cols, src.rows));
    return dst;
}



Mat rotate_notcrop(Mat src, double angle)
{
    Mat dst;
    Point2f pt(src.cols/2., src.rows/2.);    
    Mat r = getRotationMatrix2D(pt, angle, 1.0);

    cv::Point2f center(src.cols/2.0, src.rows/2.0);
    cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);
    // determine bounding rectangle
    cv::Rect bbox = cv::RotatedRect(center,src.size(), angle).boundingRect();
    // adjust transformation matrix
    rot.at<double>(0,2) += bbox.width/2.0 - center.x;
    rot.at<double>(1,2) += bbox.height/2.0 - center.y;

    warpAffine(src, dst, rot, bbox.size());
    return dst;
}


string getIBAN(tesseract::TessBaseAPI *api, Boxa* box){
    
    for (int z = 0; z < box->n; z++) {

        BOX* b = boxaGetBox(box, z, L_CLONE);
        api->SetRectangle(b->x, b->y, b->w, b->h);
        char* text = api->GetUTF8Text();
        int confidence = api->MeanTextConf();
        string ll(text);
        std::smatch m;
        //std::regex e ("(.*)(FR)(.*)([0-9]{2})(.+)([0-9]{4})(.+)([0-9]{4})(.*)");
        std::regex e ("(.*)(FR)(.*)([0-9]{2})(.+)([0-9]{4})(.+)([0-9]{4})(.*)");
        if(std::regex_search (ll,m,e))
            {
            return ll + ": confidence :" + std::to_string(confidence);
            }

    }
    return "";

}

string getName(tesseract::TessBaseAPI *api, Boxa* box){
     for (int z = 0; z < box->n; z++) {

        BOX* b = boxaGetBox(box, z, L_CLONE);
        api->SetRectangle(b->x, b->y, b->w, b->h);
        char* text = api->GetUTF8Text();
        int confidence = api->MeanTextConf();
        string ll(text);
        transform(ll.begin(), ll.end(), ll.begin(), ::toupper);
        std::smatch m;
        std::regex e ("(MADEMOISELLE)|(MMME)|(MR )|(M\\.)|(MONSIEUR )|(MADAME )|(ASS )");
        //cout<<ll;
        //std::regex e ("^(M. )|(MMME )|(M )|(MONSIEUR )|(MR. )|(MADEMOISELLE )+$");
        if(std::regex_search (ll,m,e))
        {
            cout<<ll<<endl;
            return ll + ": confidence :" + std::to_string(confidence);
        }

    }
    return "";
}





int main(int argc, char* argv[])
{
    char *outText;
    RIB r;

    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
    // Initialize tesseract-ocr with English, without specifying tessdata path
    if (api->Init(NULL, "eng")) {
        api->SetPageSegMode(tesseract::PSM_AUTO_OSD);
        fprintf(stderr, "Could not initialize tesseract.\n");
        exit(1);
    }

    // Open input image with leptonica library
    Pix *image;
    if(argv[1]=="")
    	image = pixRead("rib.jpg");
    else
    	image = pixRead(argv[1]);
    
    string command = "./textcleaner";
    command = command + " "  + argv[1] +" " + "out.png";
    system(command.c_str());
    image = pixRead("out.png");
    api->SetImage(image);
    // Get OCR result
    outText = api->GetUTF8Text();
    Boxa* box = api->GetComponentImages(tesseract::RIL_TEXTLINE, true, NULL, NULL);	
    //cout<< "IBAN: " << getIBAN(api, box)<<endl;
    //cout<<"Name: "<<getName(api, box)<<endl;


    getIBAN_Name(api,box,r);
    int angle=30;
    RIB max;
    max=r;
    Mat src = imread(argv[1]);

    while(angle < 360 && max.IB_confidence < 80){
        
        Mat dst;
        dst = rotate_notcrop(src, angle);
        imwrite("tmp.png",dst);
        image=pixRead("tmp.png");

        api->SetImage(image);
        outText = api->GetUTF8Text();
        Boxa* box = api->GetComponentImages(tesseract::RIL_TEXTLINE, true, NULL, NULL); 
        getIBAN_Name(api,box,r);
        if(max.IB_confidence<r.IB_confidence)
            max=r;
        angle+=30;
        cout<<"confidence "<< max.IB_confidence<< ": angle : " << angle<< endl;


    }

    cout<<correct_IBAN(max.IB)<< ": Confidence "<< max.IB_confidence<<endl;
    cout<<max.name<<": Confidence "<< max.name_confidence<<endl;
    api->End();
    delete [] outText;
    pixDestroy(&image);
    return 0;
}