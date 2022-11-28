/*complie : 
clang++ -std=c++11 -I<headers> -L<lib>  -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc vert_gen.cpp  \
run : 
./a.out <img>
*/
#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/imgproc.hpp>

#include "opencv2/opencv.hpp"

#include "set.h"
#include "helper.h"

#include <iostream>
#include <fstream>
#include <cmath>

void formColor (const cv::Mat& in , std::vector<std::vector<int> >& inColor) {
    std::vector<std::vector<int> > temp; 
    //add all colors first 
    for (int i = 0; i < in.rows ; ++i) {
        for (int j = 2; j < in.cols*3; j+=3) {
            bool same = 0; 
            for (int a = 0; a < temp.size(); ++a) {
                if ( in.ptr<uchar>(i)[j] == temp[a][2] &&
                in.ptr<uchar>(i)[j-1] == temp[a][2-1] &&
                in.ptr<uchar>(i)[j-2] == temp[a][2-2]) {
                    same = 1; 
                    break;
                }
            }
            if (!same) temp.push_back({ in.ptr<uchar>(i)[j-2],in.ptr<uchar>(i)[j-1],in.ptr<uchar>(i)[j] });
        }
    }
    //show size 
    std::cout << temp.size() << std::endl;
    //special entry
    inColor.push_back({-1,-1,-1});
    //prompt to user to select colors 
    //1 for yes , 0 for no
    for (int a = 0; a < temp.size(); ++a) {
        bool yes = 1; 
        std::cout << "B :" << temp[a][0] << " G :" << temp[a][1] << " R :" << temp[a][2] << " ? "; 
        std::cin >> yes; 
        std::cout << std::endl; 
        if (yes) inColor.push_back({temp[a][0],temp[a][1],temp[a][2]}); 
    }
    //print out colors that will be used 
    std::cout << "Color Used :" << std::endl;
    for (int a = 0; a < inColor.size(); ++a) {
        std::cout << "B :" << inColor[a][0] << " G :" << inColor[a][1] << " R :" << inColor[a][2] << std::endl; 
    }
}


/*
@param in sample image . 3 channels BGR opencv matrix 
@param inColor desired colors from reference image . First entry should be a dummy {-1,-1,-1} .
@param sets empty . array of SET objects. 
@return sets 
*/
void formSets (const cv::Mat& in , std::vector<std::vector<int> > inColor  , std::vector<SET>& sets) {
    //![maps]
    //empty . each entry will be used to initialize a SET objects at the end
    std::vector<std::vector<std::vector<int> > > maps;
    //![maps] 
    //![colPalette] 
    SET::colPalette = inColor; 
    //![colPalette] 

    //![grouping]
        for (int i = 0; i < in.rows ; ++i) {
            //j points to the R in BGR of each pixel in a row 
            for (int j = 2; j < in.cols*3; j+=3) {
                //test 1 : check color 
                //note : suppose pixel is displayed at (x,y) then the pixel corresponds to the (y,x) entry of opencv mat , ie. mat.ptr<uchar>[y][x]
                for (int a = 1; a < SET::colPalette.size(); ++a) { 
                    if (in.ptr<uchar>(i)[j] == SET::colPalette[a][2] && 
                    in.ptr<uchar>(i)[j-1] == SET::colPalette[a][2-1] &&
                    in.ptr<uchar>(i)[j-2] == SET::colPalette[a][2-2]
                    ) {
                        //std::cout << "aye" << std::endl; 
                        //![declear merge maps]
                        //@var mm an array containing indeices of maps to be merged for each pixel 
                        std::vector<int> mm; 
                        //![declear merge maps]
                        int c = (j+1)/3-1; //column position of pixel 
                        //test 2 : check with exisiting maps 
                        for (int b = 0; b < maps.size(); ++b) {
                            //row = 0 , col > 0 ? (ignore first pixel) 
                            if (i == 0 && c > 0) {
                                //check W 
                                if (maps[b][i][c-1] != 0) {
                                    mm.push_back(b); 
                                }
                            }
                            //row > 0 , col = 0 ? 
                            else if (i > 0 && c == 0) {
                                //check N , NE
                                if (maps[b][i-1][c] != 0 || maps[b][i-1][c+1] != 0) {
                                    mm.push_back(b); 
                                }
                            }
                            //row > 0 , col = final ?
                            else if (i > 0 && c == maps[0][0].size()) {
                                //check N , NW
                                if (maps[b][i-1][c] != 0 || maps[b][i-1][c-1] != 0) {
                                    mm.push_back(b); 
                                }
                            }
                            //row > 0 , 0 < col < final ?
                            else {
                                //check W , NW , N , NE 
                                if (maps[b][i][c-1] != 0 || maps[b][i-1][c-1] != 0 || maps[b][i-1][c] != 0 || maps[b][i-1][c+1] != 0) {
                                    mm.push_back(b); 
                                }
                            }
                        }
                        //test 3 : create new , merge or add to maps
                        //create new map
                        if (mm.size() == 0) {
                            std::vector<std::vector<int> > map; // new map 
                            for (int d = 0; d < in.rows; ++d) {
                                std::vector<int> row; //new row for the new map 
                                for (int e = 0; e < in.cols ; ++e) {
                                    if (d == i && e == c) row.push_back(a); else row.push_back(0);
                                }
                                map.push_back(row); 
                            }
                            maps.push_back(map);
                        }
                        //merge maps
                        else if (mm.size() > 1) {
                            //add to first map in mm 
                            //for each row of the first map 
                            for (int f = 0; f < in.rows; ++f) {
                                //for each col of the first map
                                for (int g = 0; g < in.cols; ++g) {
                                    //for each other maps in mm
                                    for (int h = 1; h < mm.size(); ++h) {
                                        if (f == i && g == c) {
                                            maps[mm[0]][f][g] = a; 
                                            break; 
                                        } 
                                        else {
                                            maps[mm[0]][f][g] += maps[mm[h]][f][g]; 
                                        }
                                    }
                                }
                            }
                            //reprocess maps 
                            //std::cout << 'p' << std::endl;
                            for (int h = 1; h < mm.size(); ++h) {
                                maps.erase(maps.begin() + mm[h] - h + 1);
                            } 
                            //std::cout << 'n' << std::endl;
                        }
                        //mm.size() = 1 ; add to map 
                        else {
                            maps[mm[0]][i][c] = a;
                        }
                        break; 
                    } 
                }
            }
        }

    //![grouping]
    //![instantiating SET objects]
    for (int k = 0; k < maps.size(); ++k) {
        //[tranposing maps[k]] . Since storage method in opencv is (y,x) [compares to image displayed] 
        std::vector<std::vector<int> > transposed =  transpose<int>(maps[k]); 
        //[tranposing maps[k]]  
        SET set(transposed); 
        sets.push_back(set); 
    }
    //![instantiating SET objects]
}

int main (int argc, char* argv[]) {
    const char* filename = argv[1];
    cv::Mat in = cv::imread(filename);
    if (in.empty())
    {
        std::cerr << "Can't open image ["  << filename << "]" << std::endl;
        return EXIT_FAILURE;
    };
    std::vector<std::vector<int> > colors; 
    std::vector<SET> sets;

    formColor(in,colors);  
    

    formSets(in,colors,sets); 

   std::cout <<  sets[0].angleData[0].size() << std::endl; 

    sets[0].vertex(.2*sets[0].angleData[0].size(),.2); 
    std::cout << "vertexData size : " << sets[0].vertexData[0].size() << std::endl; 
    for (int i = 0; i < sets[0].vertexData[0].size(); ++i ) {
        std::cout << "V" << i << " : " << "(" << sets[0].vertexData[0][i][0] << "," << sets[0].vertexData[0][i][1]  << ")" << std::endl; 
    }



    //display pixel by pixel 
    for (int q = 0; q < sets[0].boundaryData[0].size(); ++q ) {
        cv::Mat Z = cv::Mat::zeros(sets[0].data.size(),sets[0].data[0].size(), CV_8UC1); 
        for (int i = 0; i <= q; ++i) {
            Z.at<uchar>(sets[0].boundaryData[0][i][1] , sets[0].boundaryData[0][i][0]) = 255; 
        }
        cv::imshow("win",Z); 
        if (q%2==0)
        cv::waitKey(1);
    }


   
    cv::waitKey(0);
    cv::destroyAllWindows(); 
     
    return 0; 

}
