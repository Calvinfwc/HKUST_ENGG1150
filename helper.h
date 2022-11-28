#ifndef H_H
#define H_H 
#include <iostream>
#include <algorithm>
#include <vector>
#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/imgproc.hpp>

#include "opencv2/opencv.hpp"
#define INF 98765

/*
@param v 2d vector 
@return uchar grayscale opencv mat (imshow rquires uchar)
*/
cv::Mat vecToCVMatG (const std::vector<std::vector<int> > &v) {
    //![2d to 1d vec]
    std::vector<uchar> u; 
    for (int i = 0; i < v.size(); ++i) {
        for (int j = 0; j < v[i].size(); ++j) {
            u.push_back((v[i][j] != 0) * 255); //convert to grayscale 
        }
    }
    //![2d to 1d vec]
    //![init grayscale mat]
    //size vector 
    int row = v.size(); 
    int col = v[0].size(); 
    std::vector<int> size ({row , col}); 
    //*CV_32SC1 for int . CV_8UC1 for uchar
    //*u.data() returns pointer to first element of u . Also note std vectors have continuous storage . 
    cv::Mat GrayScaleM (size,CV_8UC1,u.data()); 
    //![init grayscale mat]
    return GrayScaleM; 
}

    /*
    @use Computes the n mean of data , ie. uniformly n partition the data , then for each partition compute the mean . The result is a vector of those means . 
    @param d 1 D data 
    @param n number of means user desires to compute , also may be the size of the vector returned 
    @return a vector of size k , 1 <= k <= n is the largest value such that (data.size()%k == 0) . 
    */
    std::vector<float> nMean(const std::vector<float>& d, int n) {
        //find k 
        while (d.size()%n != 0 && n > 1) {
            n--;
        }
        std::cout << "computing "  << n << " mean" << std::endl;
        std::vector<float> m; 
        /*
        * i is the start of each interval ie. [i,n-1] , [n,2n-1] , ... 
        * in a for loop , the comparison is carried out AFTER the increment (except at the start)
        */ 
        for (int i = 0; i < d.size() - n + 1; i += n) {
            float s = 0; 
            for (int j = 0; j < n; ++j) {
                s += d[i+j];
            }
            m.push_back(s/n); 
            // std::cout << "loop : " << i << std::endl;
        }
        return m; 
    }

    /*
    @param sample set of data for which we compute the IQR
    @param data to be determined whether it's outlier or not 
    @param threshold used for computing Q1 - threshold * IQR , Q3 + threshold * IQR
    @return true iff data is outside of Q1 - threshold * IQR or Q3 + threshold * IQR  of sample 
    @see https://en.wikipedia.org/wiki/Interquartile_range
    */
   bool isOutlier(std::vector<float> sample , float data , float threshold) {
       std::sort(sample.begin(),sample.end()); 
       int N = sample.size(); 
       /*
        *N is even : Q1 is median of {sample[0] ,..., sample[N/2-1]} of size N/2 
        *since this's symmetrical for Q3 , we just need to check whether N/2 is even or odd 
        *N is odd : Q1 is median of {sample[0] ,..., sample[(N-1)/2-1]} of size (N-1)/2 [except when N = 1]
        *since this's symmetrical for Q3 , we just need to check whether (N-1)/2 is even or odd 
       */
       float Q1 = 
       (N == 1) ? 
       /*if full array is 1 ->*/sample[0] 
        :
        /*if full array is not 1 ->*/(N%2==0) ? 

       /*if full array is even ->*/ ( (N/2) % 2 == 0 ) ? 
       /*if half array is even ->*/(sample[N/4-1] + sample[N/4])/2 : 
       /*if half array is odd ->*/sample[(N/2+1)/2 - 1] 
       : 
       /*if full array is odd ->*/( ((N-1)/2) % 2 == 0 ) ? 
       /*if half array is even ->*/( sample[(N-1)/4 - 1 ] + sample[(N-1)/4] )/2 : 
       /*if half array is odd ->*/sample[((N-1)/2 + 1) / 2 - 1];


       //trick : given an index of array that is less than the median index . the index of its "symmetrical partner" (about the median index) is array.size() - (index+1)  
       float Q3 = 
       (N == 1) ? 
       /*if full array is 1 ->*/sample[N-1] 
        :
        /*if full array is not 1 ->*/(N%2==0) ? 

       /*if full array is even ->*/ ( (N/2) % 2 == 0 ) ? 
       /*if half array is even ->*/(sample[N-N/4] + sample[N-(N/4+1)])/2 : 
       /*if half array is odd ->*/sample[N-(N/2+1)/2] 
       : 
       /*if full array is odd ->*/( ((N-1)/2) % 2 == 0 ) ? 
       /*if half array is even ->*/( sample[N-(N-1)/4 ] + sample[N-((N-1)/4+1)] )/2 : 
       /*if half array is odd ->*/sample[N - ((N-1)/2 + 1) / 2 ];

       float IQR = Q3 - Q1; 

       std::cout << "IQR = " << IQR << std::endl;

       return (data < Q1 - threshold * IQR || data > Q3 + threshold * IQR); 

   }

    /*
    @return true only if | data - mean(sample) | > threshold * mean(sample)
    */
    bool isOutlier1(std::vector<float> sample , float data , float threshold) {
        float mean = 0; 
        for (int i = 0; i < sample.size(); ++i) {
            mean += sample[i]; 
        }
        mean /= sample.size(); 

        return abs(data - mean) > threshold * mean;
    }

   /*
    @return (slope , yintersection) of regression line . EXCEPT when line is vertical , return (INF , x) . 
    @param d data 
   */
  std::vector<float> regression2D(const std::vector<std::vector<float> >& d) {
      //mean of x values 
      float xbar = 0; 
      //mean of y values 
      float ybar = 0; 
    for (int i = 0; i < d.size(); ++i) {
        xbar += d[i][0];
        ybar += d[i][1];
    }
    xbar /=  d.size(); 
    ybar /= d.size(); 
    // sample covariance : \sum{i} ( x_i - \bar{x} ) (y_i - \bar{y})
    float cov = 0; 
    // standard deviation of x : \sum_{i} ( x_i - \bar{x} )^2 
    float sdx = 0; 
    for (int i = 0; i < d.size(); ++i) {
        cov += (d[i][0] - xbar) * (d[i][1] - ybar); 
        sdx += pow((d[i][0]-xbar),2);
    }
    //INF signals to IntersectionOf2Lines() that it is a vertical line
    float slope = (sdx == 0.0f) ?  INF : cov / sdx; 
    float yintersection = (sdx == 0.0f) ? xbar /*x = constant*/ : ybar -  slope * xbar; 
    std::vector<float> regressionLine ({slope , yintersection}); 
    return regressionLine; 
  }

    /*
    @return Intersection point Of 2Lines . return {INF,INF} when no intersection . 
    @param u line 1 in the form (slope , yintersection)
    @param v line 2 in the form (slope , yintersection)
    */
    std::vector<float> IntersectionOf2Lines (const std::vector<float>& u , const std::vector<float>& v) {
        if (u[0] - v[0] == 0.0f /*parallel*/) return {INF,INF}; 
        if (u[0] == INF) return {u[1] , v[0] * u[1] + v[1]}; 
        if (v[0] == INF) return {v[1] , u[0] * v[1] + u[1]};
        float x = ( v[1] - u[1] )/(u[0] - v[0]); 
        return {x , u[0] * x + u[1] } ; 
    }

    /*
    @use for using mod to bound index 
    @return one interpretation of a%b 
    @see https://torstencurdt.com/tech/posts/modulo-of-negative-numbers/
    */
    int mod(int a, int b) {
    int c = a % b;
    return (c < 0) ? c + b : c; 
    }


    /*
    @return transpose of original 
    */
   template <class T> 
    std::vector<std::vector<T> >  transpose(const std::vector<std::vector<T> >& original) {
        std::vector<std::vector<T> > transposed; 
        //for each coloumn of original
        for (int i = 0; i < original[0].size(); ++i) {
            //row of the transposed is the column of  maps[k]
            std::vector<T> row; 
            //for each row of maps[k]
            for (int j = 0; j < original.size(); ++j) {
                row.push_back(original[j][i]);
            }
            transposed.push_back(row);
        }

        return transposed; 
    }

#endif


