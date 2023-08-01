#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;
using std::cin;
using std::cout;
using std::endl;
using std::vector;

void distance_kernel(Mat &kernel, int kernel_size);
void bilateral_filter(Mat &src, Mat &dst, Mat &kernel, int kernel_size);
void gray_scaler(Mat &src, Mat &dst);
int max_of_three(int a, int b, int c);
void ridge_edge_kernel_creator(Mat &kernel, int type);
void gaussian_blur(Mat &kernel, int kernel_size, int sigma);
float mat_summer(Mat &matrix1, int kernel_size);
void unsharper(Mat &kernel, int kernel_size, int sigma);
void box(Mat &kernel, int kernel_size);
void menu(Mat &src, Mat &dst);

float SIGMA = 75;
int KERNEL_SIZE = 5;
float UNSHARP_MULTIPLIER = -0.75;
float SMALL_NUMBER = 0.00000000001;

int main(int argc, char const *argv[])
{
    Mat image = imread("./inpimage.png", IMREAD_COLOR);
    Mat dst = image.clone();
    Mat kernel = Mat::ones(KERNEL_SIZE, KERNEL_SIZE, CV_32FC1);
    menu(image, dst);

    return 0;
}

void sigma_changer()
{
    cout << "Please enter the sigma value. For default value please send negative value";
    int temp;
    cin >> temp;
    if (temp >= 0)
    {
        SIGMA = temp;
    }
}

void kernel_size_changer()
{
    cout << "Please enter the kernel size value. For default value please send negative value";
    int temp;
    cin >> temp;
    if (temp >= 0)
    {
        KERNEL_SIZE = temp;
    }
}

void menu(Mat &src, Mat &dst)
{
    vector<String> filters = {"Gaussian", "Ridge", "Box-Blur", "Unsharping", "Bilateral"};
    // Mat kernel = Mat::ones()
    cout << "Welcome!\nWhich type of filter do you want to try?" << endl;
    for (int i = 0; i < filters.size(); i++)
    {
        cout << "[" << i + 1 << "]" << filters[i] << endl;
    }
    int selection;
    cin >> selection;

    kernel_size_changer();
    Mat kernel = Mat::ones(KERNEL_SIZE, KERNEL_SIZE, CV_32FC1);

    switch (selection)
    {
    case 1:
        sigma_changer();
        gaussian_blur(kernel, KERNEL_SIZE, SIGMA);
        filter2D(src, dst, -1, kernel);
        break;

    case 2:
        ridge_edge_kernel_creator(kernel, 0);
        filter2D(src, dst, -1, kernel);
        break;
    case 3:
        box(kernel, KERNEL_SIZE);
        filter2D(src, dst, -1, kernel);
        break;
    case 4:
        sigma_changer();
        unsharper(kernel, KERNEL_SIZE, SIGMA);
        filter2D(src, dst, -1, kernel);
        break;
    case 5:
        bilateral_filter(src, dst, kernel, KERNEL_SIZE);
        break;

    default:
        cout << "Please Enter a Valid number" << endl;
        break;
    }
    imshow("new_image", dst);
    imshow("ex_image", src);
    waitKey(0);
    destroyAllWindows();
}

void distance_kernel(Mat &kernel, int kernel_size)
{
    int mid = kernel_size / 2;
    float gauss_distance;

    float power_g;

    float w = 0;
    for (int i = 0; i < kernel.cols; i++)
    {
        for (int j = 0; j < kernel.rows; j++)
        {
            // gauss : e^((x2-x1)^2+...)/sigma)
            power_g = (pow(i - mid, 2) + pow(j - mid, 2)) / (2 * pow(SIGMA, 2));

            gauss_distance = exp(-1 * power_g);
            kernel.at<float>(i, j) = gauss_distance;
        }
    }
    cout << kernel << endl;
}

void color_kernel(Mat &src, Mat &kernel, int kernel_size, Point main_point)
{
    float r, g, b;
    float m_r, m_g, m_b;
    float color_diff, gauss_distance;
    float w = 0;
    int main = kernel_size / 2;
    Vec3b real_color = src.at<Vec3b>(main_point.x, main_point.y);
    Vec3b other_color;
    m_r = real_color[0];
    m_g = real_color[1];
    m_b = real_color[2];
    for (int i = -1 * main; i <= main; i++)
    {
        for (int j = -1 * main; j <= main; j++)
        {
            if (j + main_point.y < 0 || j + main_point.y >= src.cols || i + main_point.x < 0 || i + main_point.x >= src.rows)
            {
            }
            else
            {
                other_color = src.at<Vec3b>(main_point.x + i, main_point.y + j);
                r = other_color[0];
                g = other_color[1];
                b = other_color[2];

                color_diff = (pow(r - m_r, 2) + pow(g - m_g, 2) + pow(b - m_b, 2)) / (2 * pow(SIGMA, 2));
                gauss_distance = exp(-1 * color_diff);
                kernel.at<float>(i + main, j + main) = gauss_distance;
            }
        }
    }
}

void mat_multiplier(Mat &src, Mat &other)
{
    for (int i = 0; i < src.cols; i++)
    {
        for (int j = 0; j < src.rows; j++)
        {
            src.at<float>(i, j) *= other.at<float>(i, j);
        }
    }
}

void bilateral_filter(Mat &src, Mat &dst, Mat &kernel, int kernel_size)
{
    distance_kernel(kernel, kernel_size);

    Mat dst_kernel = kernel.clone();
    Mat clr_kernel = kernel.clone();

    cout << kernel << endl;
    int main = kernel_size / 2;
    int x, y;
    int counter = 0;
    for (int i = 0; i < src.cols; i++)
    {
        for (int j = 0; j < src.rows; j++)
        {
            dst_kernel = kernel.clone();
            color_kernel(src, clr_kernel, kernel_size, Point(i, j));
            mat_multiplier(dst_kernel, clr_kernel);
            counter++;
            vector<int> tot = vector({0, 0, 0});
            float divider = 0;
            for (int z = -1 * main; z <= main; z++)
            {
                for (int w = -1 * main; w <= main; w++)
                {
                    if (j + w < 0 || j + w >= src.cols || i + z < 0 || i + z >= src.rows)
                    {
                    }
                    else
                    {
                        Vec3b ex_color = src.at<Vec3b>(i + z, j + w);
                        float ml = dst_kernel.at<float>(z + main, w + main);

                        tot[0] += (int)(ml * ex_color[0]);
                        tot[1] += (int)(ml * ex_color[1]);
                        tot[2] += (int)(ml * ex_color[2]);
                        divider += ml;
                    }
                }
            }

            dst.at<Vec3b>(i, j) = Vec3b(tot[0] / divider, tot[1] / divider, tot[2] / divider);
        }
    }
}

float mat_summer(Mat &matrix1, int kernel_size)
{
    float tot = 0;
    for (int i = 0; i < kernel_size; i++)
    {
        for (int j = 0; j < kernel_size; j++)
        {
            tot += matrix1.at<float>(i, j);
        }
    }
    return tot;
}

int max_of_three(int a, int b, int c)
{
    int max = a > b ? (a > c ? a : c) : (b > c ? b : c);
    return max;
}

int min_of_three(int a, int b, int c)
{
    return a < b ? (a < c ? a : c) : (b < c ? b : c);
}

int mid_of_three(int a, int b, int c)
{
    return a < b ? (a < c ? c : a) : (b < c ? c : b);
}

void gray_scaler(Mat &src, Mat &dst)
{
    for (int i = 0; i < src.cols; i++)
    {
        for (int j = 0; j < src.rows; j++)
        {
            Vec3b ex_color = src.at<Vec3b>(i, j);
            int new_code = min_of_three(ex_color[0], ex_color[1], ex_color[2]);
            dst.at<Vec3b>(i, j) = Vec3b(new_code, new_code, new_code);
        }
    }
    imshow("ex_image", src);
    imshow("new_image", dst);
    waitKey(0);
    destroyAllWindows();
}

void identity_kernel_creator(Mat &dst)
{
    int mid = dst.cols / 2;
    for (int i = 0; i < dst.cols; i++)
    {
        for (int j = 0; j < dst.rows; i++)
        {
            if (i == mid and j == mid)
            {
                dst.at<float>(i, j) = 1;
            }
            else
            {
                dst.at<float>(i, j) = 0;
            }
        }
    }
}

void ridge_edge_kernel_creator(Mat &kernel, int type)
{

    int mid = KERNEL_SIZE / 2;
    if (type)
    {
        for (int i = 0; i < kernel.cols; i++)
        {
            for (int j = 0; j < kernel.rows; j++)
            {
                if (i == mid and j == mid)
                {
                    kernel.at<float>(i, j) = 4 * mid;
                }
                else if (i == j)
                {
                    kernel.at<float>(i, j) = -1;
                }
                else
                {
                    kernel.at<float>(i, j) = 0;
                }
            }
        }
    }
    else
    {
        for (int i = 0; i < kernel.cols; i++)
        {
            for (int j = 0; j < kernel.rows; j++)
            {
                if (i == mid and j == mid)
                {
                    kernel.at<float>(i, j) = pow(KERNEL_SIZE, 2) - 1;
                }
                else
                {
                    kernel.at<float>(i, j) = -1;
                }
            }
        }
    }
}

void gaussian_blur(Mat &kernel, int kernel_size, int sigma)
{
    float res;
    int mid = kernel_size / 2;
    float tot = 0;
    cout << "Gaussian kernel preparing" << endl;
    for (int i = 0; i < kernel_size; i++)
    {
        for (int j = 0; j < kernel_size; j++)

        {
            res = exp(-1 * ((pow(mid - i, 2) + pow(mid - j, 2)) / (2 * pow(sigma, 2)))) / (sqrt(2 * M_PI * pow(sigma, 2)));
            kernel.at<float>(i, j) = res;
            tot += res;
        }
    }
    kernel /= tot;
    cout << kernel << endl;
    cout << "Gaussian kernel has been prepared, image filtering..." << endl;
}

void unsharper(Mat &kernel, int kernel_size, int sigma)
{
    gaussian_blur(kernel, kernel_size, sigma);
    kernel.at<float>(kernel_size / 2, kernel_size / 2) = UNSHARP_MULTIPLIER * mat_summer(kernel, kernel_size);
}

void box(Mat &kernel, int kernel_size)
{
    int tot = 0;
    kernel = Mat::ones(kernel_size, kernel_size, CV_32F);
    tot = mat_summer(kernel, kernel_size);
    kernel /= tot;
}