#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include<iostream>
#include <sstream>
using namespace std;

int main(int argc, char* argv[])
{
    char *outText;

    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
    // Initialize tesseract-ocr with English, without specifying tessdata path
    if (api->Init(NULL, "eng")) {
        fprintf(stderr, "Could not initialize tesseract.\n");
        exit(1);
    }

    // Open input image with leptonica library
    Pix *image;
    if(argv[1]!="")
    	image = pixRead("rib.jpg");
    else
    	image = pixRead(argv[1]);

    api->SetImage(image);

    // Get OCR result
    outText = api->GetUTF8Text();
    string line;

    stringstream f;
    f<<outText;
    string IBAN;
    while(getline(f,line)!=NULL)
    {
    	//cout<<line<<endl;
    	if(line.find("IBAN")){
    		//cout<<line;
    		IBAN=line;
    	}


    }



    Boxa* box = api->GetComponentImages(tesseract::RIL_TEXTLINE, true, NULL,
   NULL);

	std::cout << box->n << " text line" << std::endl;

 	for (int z = 0; z < box->n; z++) {

	BOX* b = boxaGetBox(box, z, L_CLONE);
	api->SetRectangle(b->x, b->y, b->w, b->h);
	char* text = api->GetUTF8Text();
	int confidence = api->MeanTextConf();
	string ll(text);
	if(ll.find("IBAN")!=std::string::npos)
	  std::cout << "BOX " << z << " (Y X H W) (" << b->y << " " << b->x << " "
	    << b->h << " " << b->w << ") CONFIDENCE " << confidence
	    << " TEXT > " << text << std::endl;
	}
 
    //printf("OCR output:\n%s", outText);

    tesseract::ResultIterator* ri = api->GetIterator();
    tesseract::PageIteratorLevel level = tesseract::RIL_WORD;

    if (ri != 0) {
        do {
            const char* symbol = ri->GetUTF8Text(level);
            float conf = ri->Confidence(level);
            if (symbol != 0) {
                //printf("symbol %s, conf: %f", symbol, conf);
                bool indent = false;
                tesseract::ChoiceIterator ci(*ri);
                do {
                    if (indent)
                        printf("\t \t \t");
                    const char* choice = ci.GetUTF8Text();
                    //printf("\t- %s conf: %f\n", choice, ci.Confidence());
                    indent = true;
                } while (ci.Next());
            }
            printf("---------------------------------------------\n");
            delete[] symbol;
        } while ((ri->Next(level)));
    }

    // Destroy used object and release memory
    api->End();
    delete [] outText;
    pixDestroy(&image);
    cout<<"Finish"<<endl;
    return 0;
}