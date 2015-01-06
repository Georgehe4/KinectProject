##KinectProject
=============

#Kinect Stitching
Create by George He and Ashwin Chetty - Carleton College Summer 2013

#Introduction
This project aims to create a realistic 3D point cloud rendering of the real world using the Microsoft Kinect.
Programs: C++, Microsoft Kinect SDK, OpenGL
[View on youtube](https://www.youtube.com/watch?v=aTB151ramo8&)

The program is broken into the following components:
1. Video Intake
2. Geometric segmentation of frames using random sample consensus (RANSAC)
3. Single-frame definition using flood-fill, starting from RANSAC identified shapes
4. Frame-by-frame comparison to calculate angle deviation between each frame
5. Overlaying frames into singular rendering.


