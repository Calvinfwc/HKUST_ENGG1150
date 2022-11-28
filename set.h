#ifndef SET_H
#define SET_H 
#include <iostream>
#include <vector>
#include <cmath>


extern std::vector<float> nMean(const std::vector<float>& d, int n); 
extern std::vector<float> regression2D(const std::vector<std::vector<float> >& d); 
extern std::vector<float> IntersectionOf2Lines (const std::vector<float>& u , const std::vector<float>& v); 
extern bool isOutlier(std::vector<float> sample , float data , float threshold); 
extern bool isOutlier1(std::vector<float> sample , float data , float threshold) ; 
extern int mod(int a, int b); 

class SET {
    public:
    //how to read data . {{-1,-1,-1} , {BGR} , ... , {BGR}} . {-1,-1,-1} means color is not in colPalette
    static std::vector<std::vector<int> > colPalette; 
    // a color map . value of entry is an index to colPalette 
    std::vector<std::vector<int> >  data;

    //outer boundary only . 3D array , for each color for each x for each y . 
    std::vector<std::vector<std::vector<float> > >  boundaryData;
    std::vector<std::vector<float> >  angleData;
    std::vector<std::vector<std::vector<float> > > vertexData; 

    SET(std::vector<std::vector<int> > in);
  
//    void loadBoundary(); 
    void boundary();  
    void reba(int n);

    cv::Mat boundaryCVMatG(int p); 

    void sortBoundary(); 

    void angle(); 

    void vertex(int n, float t);



};

std::vector<std::vector<int> > SET::colPalette; // this is how you initialize static class member . You do it outside of class . 

/*
 @param in : in should be a pixel matrix (no subpixels !) . -1 in an entry if the respective pixel is not in colPalette . otherwise 
 entry should take that color value . 
*/
SET::SET(std::vector<std::vector<int> >  in) : data(in) , boundaryData() , angleData() , vertexData() {
    boundary(); 
    sortBoundary(); 
    angle();
}

void SET::boundary() {
    //for each row of boundaryData
    for (int i = 1; i < colPalette.size(); ++i ) {
        std::vector<std::vector<float> > row; 
        //the current color is colPalette[i] 
            //for each row of data
                for (int x = 0; x < data.size(); ++x) {
                    //for each col of data
                            for (int y = 0; y < data[x].size(); ++y) {
                                if ( data[x][y] == i ) {
                                    //those must be boundary
                                    if (x == 0 || x == data.size() - 1 || y == 0 || y == data[x].size() - 1) {
                                        row.push_back({(float) x,(float) y});
                                    } 
                                    //check NW , W , SW , S , SE , E , NE , N
                                    else if ( data[x-1][y-1] == 0 || data[x][y-1] == 0 || data[x+1][y-1] == 0 || data[x+1][y] == 0 ||
                                    data[x+1][y+1] == 0 || data[x][y+1] == 0 || data[x-1][y+1] == 0 || data[x-1][y] == 0 ) {
                                        row.push_back({(float) x,(float) y}); 
                                    }
                                }
                            }
                }
                boundaryData.push_back(row); 
    }
}

/*
@param p index to colPalette
@return uchar opencv Grayscale Mat for that boundary 
*/
cv::Mat SET::boundaryCVMatG(int p) {
    cv::Mat Z = cv::Mat::zeros(data.size(),data[0].size(), CV_8UC1); 

    if (p < boundaryData.size()) {
        for (int i = 0; i < boundaryData[p].size(); ++i) {
            Z.at<uchar>(boundaryData[p][i][1],boundaryData[p][i][0]) = 255; 
        }
    }
    return Z; 
}



/*
@param p index to colPalette . -1 to sort all. 
@return void . But modifies boundary such that adjacent vertices are next to each other .  
*/
void SET::sortBoundary() {
    //sort all 
    for (int p = 0; p < boundaryData.size(); ++p) {
        //for each data 
        for (int a = 0; a < boundaryData[p].size() - 1; ++a) {
            int argmin = a+1; //boundaryData[p][argmin] that is closest to boundaryData[p][a]
            //find closest dot
            for (int i = a+2; i < boundaryData[p].size(); ++i) {
            //squared euclidean distance (comparing with boundaryData[p][argmin])
                float argminDis = pow(boundaryData[p][argmin][0] - boundaryData[p][a][0] , 2) + pow(boundaryData[p][argmin][1] - boundaryData[p][a][1] , 2);
                float NextDis = pow(boundaryData[p][i][0] - boundaryData[p][a][0] , 2) + pow(boundaryData[p][i][1] - boundaryData[p][a][1] , 2);
                if (NextDis < argminDis) {
                    argmin = i; 
                }
            }
            //interchange 
            std::vector<float> temp = boundaryData[p][a+1];
            boundaryData[p][a+1] = boundaryData[p][argmin];
            boundaryData[p][argmin] = temp; 
        }
    }
}



void SET::angle() {
    for (int p = 0; p < boundaryData.size(); ++p) {
        std::vector<float> row; 
        for (int a = 0; a < boundaryData[p].size(); ++a) {
            //%boundaryData[p].size() to "form a cirle"
            row.push_back(
                  atan2(
                      boundaryData[p][(a+2)%boundaryData[p].size()][1] - boundaryData[p][(a+1)%boundaryData[p].size()][1] ,
                     boundaryData[p][(a+2)%boundaryData[p].size()][0] - boundaryData[p][(a+1)%boundaryData[p].size()][0] 
            ) -
            atan2(
                boundaryData[p][(a)%boundaryData[p].size()][1] - boundaryData[p][(a+1)%boundaryData[p].size()][1] ,
                boundaryData[p][(a)%boundaryData[p].size()][0] - boundaryData[p][(a+1)%boundaryData[p].size()][0] 
            ) 
            ); 
        }
        angleData.push_back(row); 
    }
}

/*
@use recompute boundary and angle data (from current boundary data)
@param n for nMean()
*/
void SET::reba(int n) {
    //![boundary]
    for (int p = 0; p < boundaryData.size(); ++p) {
        std::vector<float> x;
        std::vector<float> y;
        std::vector<std::vector<float> > row;
        //load x , y
        for (int i = 0; i < boundaryData[p].size(); ++i) {
            x.push_back(boundaryData[p][i][0]);
            y.push_back(boundaryData[p][i][1]);
        }
        //store recomputation in x , y
        x = nMean(x,n);
        y = nMean(y,n);
        //reformat x , y data as row
        for (int i = 0; i < x.size(); ++i) {
            row.push_back({x[i] , y[i]});
        }
        //reassignment 
        boundaryData[p] = row;
    }
    //![boundary]
    //![angle]
    angleData.clear(); 
    angle();
    //![angle]
} 



void SET::vertex(int n , float t) {
    //for each color 
    for (int p = 0; p < angleData.size(); ++p) {
        std::vector<std::vector<float> > row; 
        int i = 0; //index to angleData 

    //[debug]
     std::vector<float> ang; 
    for (int k = 0 ; k < angleData[p].size() ; ++ k ) {
        ang.push_back(abs( angleData[p][k] ) ); 
    }
    //[debug]

        while (i < angleData[p].size()) {
            std::cout << "data : " << ang[i] << std::endl; 
            if (isOutlier1(ang,abs(angleData[p][i]),t)) {
                //[prep data]
                std::vector<std::vector<float> > LEFT; 
                for (int a = 0; a < n; ++a) {
                    LEFT.push_back(boundaryData[p][ mod(i-a,angleData[p].size())  ]); //instead of (i-a)%angleData[p].size()
                     std::cout <<"LEFT :" << std::endl; 
                    std::cout <<"i :" << i << ',' << std::endl; 
                    std::cout <<"a :" << a << ',' << std::endl;  
                    std::cout <<"size :" << angleData[p].size() << ',' << std::endl; 
                    std::cout <<"mod :" << mod(i-a,angleData[p].size()) << std::endl; 
                    std::cout <<"val :" << "(" << boundaryData[p][ mod(i-a,angleData[p].size())  ][0] << "," <<
                    boundaryData[p][ mod(i-a,angleData[p].size())  ][1] << ")"
                    << std::endl; 
                }
                std::cout << std::endl; 
                std::vector<std::vector<float> > RIGHT; 
                for (int a = 1; a < n+1; ++a) {
                    RIGHT.push_back(boundaryData[p][ mod(i+a,angleData[p].size()) ]); // instead of  (i+a)%angleData[p].size()
                                         std::cout <<"RIGHT :" << std::endl; 
                    std::cout <<"i :" << i << ',' << std::endl;
                    std::cout <<"a :" << a << ',' << std::endl;  
                    std::cout <<"size :" << angleData[p].size() << ',' << std::endl; 
                    std::cout <<"mod :" << mod(i+a,angleData[p].size()) << std::endl;  
                    std::cout <<"val :" << "(" << boundaryData[p][ mod(i+a,angleData[p].size())  ][0] << "," <<
                    boundaryData[p][ mod(i+a,angleData[p].size())  ][1] << ")"
                    << std::endl; 
                }
                //[prep data] 
                //[regression] 
                std::vector<float> LINE0 = regression2D(LEFT); 
                std::vector<float> LINE1 = regression2D(RIGHT); 
                std::cout << "SLOPE0 : " << LINE0[0] << "inter0 : " << LINE0[1]  << std::endl; 
                std::cout << "SLOPE1 : " << LINE1[0] << "inter1 : " << LINE1[1]  << std::endl; 
                //[regression] 
                //[vertex] 
                row.push_back(IntersectionOf2Lines(LINE0,LINE1));
                //[vertex] 
                //[update i]
                i += (n+1) ; 
                //[update i]
            } else {
                ++i; 
            } 
        }
        vertexData.push_back(row); 
    }
}






#endif 
