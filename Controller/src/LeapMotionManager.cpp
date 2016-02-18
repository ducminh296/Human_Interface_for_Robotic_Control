//*****************************************************************************
//
// LeapMotionManager.cpp
//
// Class responsible for interfacing with the Leap Motion Controller.
//
// Copyright (c) 2015 Brandon To, Minh Mai, and Yuzhou Liu
// This code is licensed under BSD license (see LICENSE.txt for details)
//
// Created:
// December 28, 2015
//
// Modified:
// Feburary 18, 2016
//
//*****************************************************************************
#include "LeapMotionManager.h"

#include <cassert>

//*****************************************************************************
//
//! Empty constructor for LeapMotionManager.
//!
//! \param None.
//!
//! \return None.
//
//*****************************************************************************
LeapMotionManager::LeapMotionManager()
{

}

//*****************************************************************************
//
//! Empty destructor for LeapMotionManager.
//!
//! \param None.
//!
//! \return None.
//
//*****************************************************************************
LeapMotionManager::~LeapMotionManager()
{

}

//*****************************************************************************
//
//! Processes the Leap frame. Called once every application frame.
//!
//! \param buf buffer to store processed data.
//! \param buflen the size of the buffer
//!
//! \return Returns \b true if the frame was processed successfully and
//! \b false otherwise.
//
//*****************************************************************************
bool LeapMotionManager::processFrame(LeapDataStruct leapData)
{
    _LeapAngleStruct leapAngles;

    //
    // Get current frame from Leap Motion Controller
    //
    Leap::Frame frame = _controller.frame();

    //
    // Get camera image from frame
    //
    Leap::ImageList imageList = frame.images();
    Leap::Image image = imageList[0];
    const unsigned char *imageData = image.data();
    int imageSize = image.bytesPerPixel() * image.height() * image.width();
    if (imageSize > 0)
    {
        if (imageSize != LeapDataStruct::_IMAGE_SIZE)
        {
            std::cout << "[ERROR] LeapMotionManager::processFrame(): "\
                "Unexpected image size." << std::endl;
            return false;
        }

        leapData.imageAvailable = true;

        //
        // Set image RGB to same value to display grayscale and set alpha to
        // opaque
        //
        for (int i=0; i<imageSize; i++)
        {
            leapData.imageDataRGBA[4*i] = imageData[i];
            leapData.imageDataRGBA[4*i+1] = imageData[i];
            leapData.imageDataRGBA[4*i+2] = imageData[i];
            leapData.imageDataRGBA[4*i+3] = 0xFF;
        }
    }
    else
    {
        leapData.imageAvailable = false;
    }

    //
    // Get hand data from frame
    //
    Leap::HandList hands = frame.hands();
    Leap::Hand hand;

    //
    // Iterates through list of hands provided by Leap Motion Controller and
    // identifies first right hand it detects.
    //
    for (auto it = hands.begin(); it != hands.end(); it++)
    {
        if ((*it).isRight())
        {
            hand = (*it);
        }
    }

    //
    // Checks if right hand was detected by Leap Motion Controller
    //
    if (!hand.isValid() || !hand.isRight())
    {
        std::cout << "No hand detected." << std::endl;
        unsigned int bufIndex = 0;
        for (int i = 0; i < NUM_FINGERS; i++)
        {
            //
            // Stores angle sequentially in buf
            //
            leapData.data[bufIndex++] = 0;
        }
        leapData.data[bufIndex++] = 0;
        return false;
    }

    //
    // Populates LeapDataStruct and LeapAngleStruct structure with finger info
    //
    Leap::FingerList fingers = hand.fingers();
    for (auto it = fingers.begin(); it != fingers.end(); it++)
    {
        Leap::Finger finger = (*it);
        Leap::Vector direction[4];
        for (auto b=0; b<4; b++)
        {
            Leap::Bone::Type boneType = static_cast<Leap::Bone::Type>(b);
            Leap::Bone bone = finger.bone(boneType);
            direction[b] = bone.direction();

            if (boneType == Leap::Bone::TYPE_DISTAL)
            {
                Leap::Vector center = bone.center();
                SDL_Rect &fingerRect = leapData.fingerRects[finger.type];

                //
                // Calculates position on screen to render to
                // This makes it pretty accurate... don't ask...
                //
                fingerRect.w = 25 - 25 * (center.y / 500);
                if (fingerRect.w < 5)
                {
                    fingerRect.w = 5;
                }
                fingerRect.h = fingerRect.w;
                fingerRect.x = -center.x*(170/center.y)*0.85 + 800/2
                    + 0.7*fingerRect.w;
                fingerRect.y = center.z*(170/center.y)*0.85 + 600/2
                    - 1.2*fingerRect.h;
            }
        }
        leapAngles.totalAngle[finger.type()] = (unsigned char)
            (_radiansToDegrees(_calculateTotalAngle(direction, 4)));
    }

    //
    // Populates LeapAngleStruct structure with wrist info
    //
    Leap::Vector palmNormal = hand.palmNormal();
    float rollInRadians = palmNormal.roll();
    float rollInDegrees = _radiansToDegrees(rollInRadians);
    float wristAngle = rollInDegrees + 90;
    if (wristAngle > 180)
    {
        wristAngle = 180;
    }
    else if (wristAngle < 0)
    {
        wristAngle = 0;
    }
    leapAngles.wristAngle = (unsigned char)(wristAngle);

    //
    // Serialize data
    //
    _serialize(leapAngles, leapData.data, leapData._MAX_PAYLOAD);

    return true;
}

//*****************************************************************************
//
//! Calculates the total angle (in radians) between the vectors
//!
//! \param vectors the array of vectors.
//! \param size the length of the array.
//!
//! \return Returns the total angle (in radians) between the vectors.
//
//*****************************************************************************
float LeapMotionManager::_calculateTotalAngle(Leap::Vector *vectors,
    unsigned int size)
{
    float angle = 0;

    for (unsigned int i=0; i<size-1; i++)
    {
        float temp = vectors[i].angleTo(vectors[i+1]);
        angle += temp;
    }

    return angle;
}

//*****************************************************************************
//
//! Converts an angle in radians to an angle in degrees.
//!
//! \param angle the angle in radians to convert.
//!
//! \return Returns the converted angle in degrees.
//
//*****************************************************************************
float LeapMotionManager::_radiansToDegrees(float angle)
{
    const float _RADIANS_TO_DEGREES = (float)57.2958;
    return _RADIANS_TO_DEGREES * angle;
}

//*****************************************************************************
//
//! Serializes structure of angle data and stores into buffer.
//!
//! \param leapAngles the structure with data to serialize.
//! \param buf the buffer to store serialized data.
//! \param buflen the size of the buffer
//!
//! \return None.
//
//*****************************************************************************
void LeapMotionManager::_serialize(_LeapAngleStruct &leapAngles,
    unsigned char *buf, unsigned int buflen)
{
    //
    // Buffer needs to be at least this size
    //
    const int BITS_PER_BYTE = 8;
    int angleSize = sizeof(leapAngles.totalAngle[0]);
    int wristSize = sizeof(leapAngles.wristAngle);
    assert(buflen >= (angleSize*NUM_FINGERS + wristSize));

    //
    // Serialize data
    //
    unsigned int bufIndex = 0;
    for (int i=0; i<NUM_FINGERS; i++)
    {
        auto angle = leapAngles.totalAngle[i];

        //
        // Stores angle sequentially in buf
        //
        buf[bufIndex++] = angle;
        std::cout << static_cast<unsigned int>(buf[bufIndex - 1]) << " ";
    }
    buf[bufIndex++] = leapAngles.wristAngle;
    std::cout << static_cast<unsigned int>(buf[bufIndex - 1]) << std::endl;
    std::cout << std::endl;
}
