
//@author Burak ÇİGDEM
#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <vector>
#include <cstdlib>
using namespace std;
using namespace cv;
//I girdi görüntü, M ve N görüntünün x ekseni ve y ekseni boyutları, L maksimum seviye sayısı
vector<double> histogram_hesapla(Mat I, int L,int M,int N)
{
    vector<double> hist(256,0); //vector tipinde histogram dizisi oluşturuldu.
    for (int i = 0; i < M; i++)
    {
        for (int j = 0; j < N; j++)
        {
            hist[(int)I.at<uchar>(i, j)]++; //Input görüntünün boyutları gezilerek histogram hesaplanır.
        }

    }
    return hist;
}
// hist histogram, M ve N görüntünün x ekseni ve y ekseni boyutları, L maksimum seviye sayısı

vector<double> pdf_hesapla(vector<double> hist, int L, int M, int N) {
    vector<double> pdf(256, 0);
    for (int i = 0; i < L; i++)
    {
        pdf[i] = hist[i] / (M * N); //PDF(Probability mass function) hesaplanır.
    }
    return pdf;
}

//pdf-->Olasılık Yoğunluk Fonksiyonu, L maksimum seviye sayısı

vector<double> cdf_hesapla(vector<double> pdf, int L) {
    vector<double> cdf(256, 0);
    double sum_pdf = 0;
    for (int i = 0; i < L; i++)
    {
        sum_pdf += pdf[i];
        cdf[i] = sum_pdf;
    }
    return cdf;
}
//i_cdf-->input resmin kümülatif dağılım fonksiyonu, L maksimum seviye sayısı

vector<double> tf_hesapla(vector<double> i_cdf, vector<double> ref_cdf, int L) {
    vector<double> tf(256, 0);
    double min_dif;
    for (int i = 0; i < L; i++)
    {
        min_dif = 1.2;//bunu atamamın sebebi ilk aşamada cur_dif min_dif den küçük olmasını sağlamak
        int ind = 0;
        for (int j = 0; j < L; j++) {
            double cur_dif = abs(i_cdf[i] - ref_cdf[j]); //input ve referance cdf degerlerini çıkarıp cur_dif değişkenine atadım.
            if (cur_dif < min_dif) {
                min_dif = cur_dif;//Minimum olan değeri bulmaya çalışıyor.
                ind = j;
            }
        }
        tf[i] = ind;//bulunan bu değer tf dizisine atanıyor.
    }
    return tf;
}
//I input görüntü, tf->transformation funciton, M ve N girdi görüntünün x ekseni ve y ekseni boyutları
Mat tf_uygula(Mat I, vector<double> tf, int M, int N) {
    Mat out = I.clone();
    for (int i = 0; i < M; i++)
    {
        for (int j = 0; j < N; j++)
        {
            out.at<uchar>(i, j) = (int)(tf[(int)I.at<uchar>(i, j)]);//tf değerlerini çıktı değerlerine atanıyor.
        }
    }
    return out;
}
// I girdi görüntü, ref referans görüntü, M ve N girdi görüntünün x ekseni ve y ekseni boyutları, W ve H M ve N referans görüntünün x ekseni ve y ekseni boyutları, L maksimum seviye sayısı

Mat h_benzetme(Mat I, Mat ref, int L, int M, int N, int W, int X) {
    vector<double> hist(256, 0);
    vector<double> pdf(256, 0);
    vector<double> cdf(256, 0);
    vector<double> tf(256, 0);
    Mat kanal[3];
    vector<vector<double>> i_cdf_vector(3);
    split(I, kanal);//BGR sırasına göre kanalları kanal matrisine atadık.
    for (int i = 0; i < 3; i++) // İnput görüntü için her bir kanalın hist,pdf,cdf değerleri hesaplandı.
    {
        hist = histogram_hesapla(kanal[i], L, M, N);
        pdf = pdf_hesapla(hist, L, M, N);
        cdf = cdf_hesapla(pdf, L);
        i_cdf_vector[i] = cdf;
    }
    split(ref, kanal);
    vector<vector<double>> ref_cdf_vector(3);
    for (int i = 0; i < 3; i++) // İnput görüntü için her bir kanalın hist,pdf,cdf değerleri hesaplandı.
    {
        hist = histogram_hesapla(kanal[i], L, W, X);
        pdf = pdf_hesapla(hist, L, W, X);
        cdf = cdf_hesapla(pdf, L);
        ref_cdf_vector[i] = cdf;
    }
    split(I, kanal);
    Mat kanal_y[3];//Çıktı Görüntünün kanalları bu diziye atanacak.
    split(I, kanal_y);
    for (int i = 0; i < 3; i++) {
        tf = tf_hesapla(i_cdf_vector[i], ref_cdf_vector[i], L);
        kanal_y[i] = tf_uygula(kanal[i], tf, M, N);
    }

    Mat out;
    vector<Mat> channels;
    channels.push_back(kanal_y[2]);
    channels.push_back(kanal_y[1]);
    channels.push_back(kanal_y[0]);
    merge(channels, out);//İşlenen kanallar birleştirildi.
    return out;
}

int main()
{
    Mat ref = imread("C:/Users/burak.cigdem/Desktop/resim/reference.jpg");
    Mat input = imread("C:/Users/burak.cigdem/Desktop/resim/input.jpg");
    int M = input.rows;
    int N = input.cols;
    int W = ref.rows;
    int X = ref.cols;
    int L = 256;
    Mat out = input.clone();
    out = h_benzetme(input, ref, L, M, N, W, X);
    imshow("Histogram Matching", out);
    imshow("İnput Resim", input);
    imshow("Referans Resim", ref);

    waitKey(0);
}
