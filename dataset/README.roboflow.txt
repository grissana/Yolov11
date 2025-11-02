
My First Project - v4 2025-11-01 4:08pm
==============================

This dataset was exported via roboflow.com on November 1, 2025 at 9:08 AM GMT

Roboflow is an end-to-end computer vision platform that helps you
* collaborate with your team on computer vision projects
* collect & organize images
* understand and search unstructured image data
* annotate, and create datasets
* export, train, and deploy computer vision models
* use active learning to improve your dataset over time

For state of the art Computer Vision training notebooks you can use with this dataset,
visit https://github.com/roboflow/notebooks

To find over 100k other datasets and pre-trained models, visit https://universe.roboflow.com

The dataset includes 130 images.
Objects are annotated in YOLOv11 format.

The following pre-processing was applied to each image:
* Auto-orientation of pixel data (with EXIF-orientation stripping)
* Resize to 640x640 (Stretch)

The following augmentation was applied to create 3 versions of each source image:
* 50% probability of horizontal flip
* 50% probability of vertical flip
* Randomly crop between 0 and 43 percent of the image
* Random rotation of between -14 and +14 degrees
* Random shear of between -3° to +3° horizontally and -13° to +13° vertically
* Salt and pepper noise was applied to 0.69 percent of pixels


