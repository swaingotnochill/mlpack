/**
 * @file core/data/stratified_split_data.hpp
 * @author Anush Kini
 *
 * Defines StratifiedSplit(), a utility function to split a dataset into
 * stratified training and test sets.
 *
 * mlpack is free software; you may redistribute it and/or modify it under the
 * terms of the 3-clause BSD license.  You should have received a copy of the
 * 3-clause BSD license along with mlpack.  If not, see
 * http://www.opensource.org/licenses/BSD-3-Clause for more information.
 */
 #ifndef MLPACK_CORE_DATA_STRATIFIED_SPLIT_DATA_HPP
 #define MLPACK_CORE_DATA_STRATIFIED_SPLIT_DATA_HPP

 #include <mlpack/prereqs.hpp>

 namespace mlpack {
 namespace data {
/**
* Given an input dataset and labels, stratify into a training set and test set.
* Example usage below.  This overload places the stratified dataset into the
* four output parameters given (trainData, testData, trainLabel, and testLabel).
*
* @code
* arma::mat input = loadData();
* arma::Row<size_t> label = loadLabel();
* arma::mat trainData;
* arma::mat testData;
* arma::Row<size_t> trainLabel;
* arma::Row<size_t> testLabel;
* math::RandomSeed(100); // Set the seed if you like.
*
* // Stratify the dataset into a training and test set, with 30% of the data
* // being held out for the test set.
* StratifiedSplit(input, label, trainData,
*                testData, trainLabel, testLabel, 0.3);
* @endcode
*
* @param input Input dataset to stratify.
* @param inputLabel Input labels to stratify.
* @param trainData Matrix to store training data into.
* @param testData Matrix to store test data into.
* @param trainLabel Vector to store training labels into.
* @param testLabel Vector to store test labels into.
* @param testRatio Percentage of dataset to use for test set (between 0 and 1).
* @param shuffleData If true, the sample order is shuffled; otherwise, each
*       sample is visited in linear order. (Default true.)
*/
template<typename T, typename U>
void StratifiedSplit(const arma::Mat<T>& input,
                     const arma::Row<U>& inputLabel,
                     arma::Mat<T>& trainData,
                     arma::Mat<T>& testData,
                     arma::Row<U>& trainLabel,
                     arma::Row<U>& testLabel,
                     const double testRatio,
                     const bool shuffleData = true)
{
  /*
  Basic idea:
  Let us say we have to stratify a dataset based on labels:
  0 0 0 0 0 1 1 1 1 1 1 (5 0s, 6 1s)

  Let our test ratio be 0.2:
  We visit each label and keep the count of each label in our unordered map

  Whenever we encounter a label, we calculate
  current_count*test_ratio --- labelMap[label]*testRatio and
  current_count+1 * test_ratio --- (labelMap[label]+1)*testRatio

  We then static_cast these counts to size_t to remove their decimal points.
  If in this case, our integer counts are same then we add to our train set.
  If there is a difference in counts, then we add to our test set

  Considering our example
  0 -- train set ( 0*0.2 == 1*0.2 ) (After casting)
  0 -- train set ( 1*0.2 == 2*0.2 ) (After casting)
  0 -- train set ( 2*0.2 == 3*0.2 ) (After casting)
  0 -- train set ( 3*0.2 == 4*0.2 ) (After casting)
  0 -- test set  ( 4*0.2 <  5*0.2 ) (After casting)

  1 -- train set ( 0*0.2 == 1*0.2 ) (After casting)
  1 -- train set ( 1*0.2 == 2*0.2 ) (After casting)
  1 -- train set ( 2*0.2 == 3*0.2 ) (After casting)
  1 -- train set ( 3*0.2 == 4*0.2 ) (After casting)
  1 -- test set  ( 4*0.2 <  5*0.2 ) (After casting)
  1 -- train set ( 5*0.2 == 6*0.2 ) (After casting)

  Finally
  train set,
  0 0 0 0 1 1 1 1 1 (4 0s, 5 1s)

  test set,
  0 1
  */
  arma::uvec Indexes;
  Indexes.set_size(inputLabel.n_cols);

  //Single index vector which adds train Indexes from the start and test
  //indexes from the end
  size_t trainIdx = 0;
  size_t testIdx = inputLabel.n_cols - 1;
  std::unordered_map<U, size_t> labelMap;

  if (shuffleData)
  {
    arma::uvec order = arma::shuffle(arma::linspace<arma::uvec>(
        0, input.n_cols - 1, input.n_cols));

    //visit the labels in shuffled order
    for (auto i: order)
    {
      auto label = inputLabel[i];
      if (static_cast<size_t>(labelMap[label]*testRatio) <
          static_cast<size_t>((labelMap[label]+1)*testRatio))
      {
        Indexes[testIdx] = i;
        testIdx -= 1;
      }
      else
      {
        Indexes[trainIdx] = i;
        trainIdx += 1;
      }
      labelMap[label] += 1;
    }
  }
  else //no shuffling required
  {
    for (size_t i = 0; i < inputLabel.n_cols; i++)
    {
      auto label = inputLabel[i];
      if (static_cast<size_t>(labelMap[label]*testRatio) <
          static_cast<size_t>((labelMap[label]+1)*testRatio))
      {
        Indexes[testIdx] = i;
        testIdx -= 1;
      }
      else
      {
        Indexes[trainIdx] = i;
        trainIdx += 1;
      }
      labelMap[label] += 1;
    }
  }
  labelMap.clear();
  testData = input.cols(Indexes.subvec(trainIdx, Indexes.n_rows-1));
  testLabel = inputLabel.cols(Indexes.subvec(trainIdx, Indexes.n_rows-1));
  trainData = input.cols(Indexes.subvec(0, trainIdx-1));
  trainLabel = inputLabel.cols(Indexes.subvec(0, trainIdx-1));
}

/**
 * Given an input dataset and labels, stratify into a training set and test set.
 * Example usage below.  This overload returns the stratified dataset as a
 * std::tuple with four elements: an arma::Mat<T> containing the training data,
 * an arma::Mat<T> containing the test data, an arma::Row<U> containing the
 * training labels, and an arma::Row<U> containing the test labels.
 *
 * @code
 * arma::mat input = loadData();
 * arma::Row<size_t> label = loadLabel();
 * auto splitResult = StratifiedSplit(input, label, 0.2);
 * @endcode
 *
 * @param input Input dataset to stratify.
 * @param inputLabel Input labels to stratify.
 * @param testRatio Percentage of dataset to use for test set (between 0 and 1).
 * @param shuffleData If true, the sample order is shuffled; otherwise, each
 *       sample is visited in linear order. (Default true).
 * @return std::tuple containing trainData (arma::Mat<T>), testData
 *      (arma::Mat<T>), trainLabel (arma::Row<U>), and testLabel (arma::Row<U>).
 */
template<typename T, typename U>
std::tuple<arma::Mat<T>, arma::Mat<T>, arma::Row<U>, arma::Row<U>>
StratifiedSplit(const arma::Mat<T>& input,
                const arma::Row<U>& inputLabel,
                const double testRatio,
                const bool shuffleData = true)
{
  arma::Mat<T> trainData;
  arma::Mat<T> testData;
  arma::Row<U> trainLabel;
  arma::Row<U> testLabel;

  StratifiedSplit(input, inputLabel, trainData, testData, trainLabel, testLabel,
      testRatio, shuffleData);

  return std::make_tuple(std::move(trainData),
                         std::move(testData),
                         std::move(trainLabel),
                         std::move(testLabel));
}

} // namespace data
} // namespace mlpack

#endif
