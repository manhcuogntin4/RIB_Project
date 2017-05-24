#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include<iostream>
#include <sstream>
#include <regex>
#include<string>
#include <algorithm>
using namespace std;
using namespace cv;


typedef struct RIB{

    string IB;
    string name;
    int IB_confidence;
    int name_confidence;
} ;


RIB getIBAN_Name(tesseract::TessBaseAPI *api, Boxa* box){
    RIB r;
    r.IB="";
    r.IB_confidence=0;
    r.name="";
    r.name_confidence=0;
    for (int z = 0; z < box->n; z++) {

        BOX* b = boxaGetBox(box, z, L_CLONE);
        api->SetRectangle(b->x, b->y, b->w, b->h);
        char* text = api->GetUTF8Text();
        int confidence = api->MeanTextConf();
        string ll(text);
        std::smatch m;
        std::regex e1 ("(.*)(FR)(.*)([0-9]{2})(.+)([0-9]{4})(.+)([0-9]{4})(.*)");
        std::regex e2 ("(MADEMOISELLE)|(MMME)|(MR )|(M\\.)|(M )|(MONSIEUR )|(MADAME )|(ASS )");
        if(std::regex_search (ll,m,e1))
        {
            r.IB=ll;
            r.IB_confidence=confidence;
        }
        if(std::regex_search (ll,m,e2))
        {
            r.name=ll;
            r.name_confidence=confidence;
        }

    }
    return r;

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
        fprintf(stderr, "Could not initialize tesseract.\n");
        exit(1);
    }

    // Open input image with leptonica library
    Pix *image;
    if(argv[1]=="")
    	image = pixRead("rib.jpg");
    else
    	image = pixRead(argv[1]);
    api->SetImage(image);
    // Get OCR result
    outText = api->GetUTF8Text();
    Boxa* box = api->GetComponentImages(tesseract::RIL_TEXTLINE, true, NULL, NULL);	
    //cout<< "IBAN: " << getIBAN(api, box)<<endl;
    //cout<<"Name: "<<getName(api, box)<<endl;


    r=getIBAN_Name(api,box);
    int angle=30;
    RIB max;
    max=r;

    while(angle <360 and max.IB_confidence<0.8){
        Mat src = imread(argv[1]);
        Mat dst;
        dst = rotate_notcrop(src, angle);
        imwrite("tmp.png",dst);
        image=pixRead("tmp.png");

        api->SetImage(image);
        outText = api->GetUTF8Text();
        Boxa* box = api->GetComponentImages(tesseract::RIL_TEXTLINE, true, NULL, NULL); 
        r=getIBAN_Name(api,box);
        angle+=30;
        if(max.IB_confidence<r.IB_confidence)
            max=r;


    }
    cout<<max.IB<< ": Confidence "<< max.IB_confidence<<endl;
    cout<<max.name<<": Confidence "<< max.name_confidence<<endl;

    api->End();
    delete [] outText;
    pixDestroy(&image);
    return 0;
}