# About
The project is about making image filters by using opencv.
## Reqirements
At debian based linux first you need to do :
```
    sudo apt install libopencv-dev
```
This will install the opencv library to your path.

You will need to cmake to build the program.

```
    sudo apt install cmake
    sudo apt install make
```

Second one might be unnecessary. Now you are ready! Let's build and execute.


## Build and Execute
Be sure about that you are in the project base path. Your path need to be like '.../opencv_filters'. Then execute 

```
    cmake -B ./build/
    make -C ./build/
```

Now you are ready to execute!

```
    ./build/DisplayImage
```

You can change the inpimage.png if you want to tet your own images. Just be sure the name of image is same! Have fun!

