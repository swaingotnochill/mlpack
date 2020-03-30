/**
 * @file image_converter_main.cpp
 * @author Jeffin Sam
 *
 * A CLI executable to load and save a image dataset.
 *
 * mlpack is free software; you may redistribute it and/or modify it under the
 * terms of the 3-clause BSD license.  You should have received a copy of the
 * 3-clause BSD license along with mlpack.  If not, see
 * http://www.opensource.org/licenses/BSD-3-Clause for more information.
 */
#include <mlpack/prereqs.hpp>
#include <mlpack/core/util/mlpack_main.hpp>
#include <mlpack/core/util/cli.hpp>
#include <mlpack/core.hpp>

using namespace mlpack;
using namespace mlpack::util;
using namespace arma;
using namespace std;
using namespace mlpack::data;


PROGRAM_INFO("Image Converter",
    // Short description.
    "A utility to load an image or set of images into a single dataset that"
    "can then be used by other mlpack methods and utilities. This can also"
    "unpack an image dataset into individual files.",
    // Long description.
    "This utility takes a image or an array of images and loads them to a"
    " matrix. You can optionally specify the height " +
    PRINT_PARAM_STRING("height") + " width " + PRINT_PARAM_STRING("width")
    + " and channel " + PRINT_PARAM_STRING("channel") + " of the images that"
    "needs to be loaded; otherwise, these parameters will be automatically"
    "detected from the image."
    "\n"
    "There are other options too, that can be specified such as " +
    PRINT_PARAM_STRING("quality")
    + ".\n\n" +
    "You can also provide a dataset and save them as images using " +
    PRINT_PARAM_STRING("dataset") + " and " + PRINT_PARAM_STRING("save") +
    " as an parameter. An example to load an image : "  +
    "\n\n" +
    PRINT_CALL("image_converter", "input", "X", "height", 256, "width", 256,
        "channel", 3, "output", "Y") +
    "\n\n" +
    " An example to save an image is :" +
    "\n\n" +
    PRINT_CALL("image_converter", "input", "X", "height", 256, "width", 256,
        "channel", 3, "dataset", "Y", "save", true),
    SEE_ALSO("@preprocess_binarize", "#preprocess_binarize"),
    SEE_ALSO("@preprocess_describe", "#preprocess_describe"),
    SEE_ALSO("@preprocess_imputer", "#preprocess_imputer"));

// DEFINE PARAM
PARAM_VECTOR_IN_REQ(string, "input", "Image filenames which have to "
    "be loaded/saved.", "i");

PARAM_INT_IN("width", "Width of the image.", "w", 256);
PARAM_INT_IN("channel", "Number of channel.", "c",  3);

PARAM_MATRIX_OUT("output", "Matrix to save images data to, Only"
    "needed if you are specifing save option.", "o");

PARAM_INT_IN("quality", "Compression of the image if saved as jpg (0-100).",
    "q", 90);

PARAM_INT_IN("height", "Height of the images.", "H", 256);
PARAM_FLAG("save", "Save a dataset as images.", "s");
PARAM_MATRIX_IN("dataset", "Input matrix to save as images.", "I");

static void mlpackMain()
{
  Timer::Start("Loading/Saving Image");
  // Parse command line options.
  const vector<string> fileNames = CLI::GetParam<vector<string> >("input");
  arma::mat out;
 
  if (!CLI::HasParam("save"))
  {
    data::ImageInfo info;
    Load(fileNames, out, info, true);
    if (CLI::HasParam("output"))
      CLI::GetParam<arma::mat>("output") = std::move(out);
  }
  else
  {
    if (!CLI::HasParam("width") || !CLI::HasParam("height") ||
        !CLI::HasParam("channel"))
    {
      throw std::runtime_error("Please provide height, width and "
          "number of channels of the images.");
    }
    // Positive value for width.
    RequireParamValue<int>("width", [](int x) { return x >= 0;}, true,
        "width must be positive");
    // Positive value for height.
    RequireParamValue<int>("height", [](int x) { return x >= 0;}, true,
        "height must be positive");
    // Positive value for channel.
    RequireParamValue<int>("channel", [](int x) { return x >= 0;}, true,
        "channel must be positive");
    // Positive value for quality.
    RequireParamValue<int>("quality", [](int x) { return x >= 0;}, true,
        "quality must be positive");

    const size_t height = CLI::GetParam<int>("height");
    const size_t width = CLI::GetParam<int>("width");
    const size_t channel = CLI::GetParam<int>("channel");
    const size_t quality = CLI::GetParam<int>("quality");
    data::ImageInfo info(width, height, channel, quality);
    if (!CLI::HasParam("dataset"))
    {
      throw std::runtime_error("Please provide a input matrix to save "
          "images from");
    }
    Save(fileNames, CLI::GetParam<arma::mat>("dataset"), info, true);
  }
}

