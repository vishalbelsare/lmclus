## LMCLUS is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## See <http://www.gnu.org/licenses/>.

setClass(
    Class="lmclus.params",
    representation=representation(
        maxDim = "numeric",
        numberOfClusters = "numeric",
        noiseSize  = "numeric",
        bestBound  = "numeric",
        errorBound = "numeric",

        hisSampling = "logical",
        hisConstSize = "numeric",
        maxBinPortion = "numeric",

        sampleHeuristic = "numeric",
        sampleFactor  = "numeric",
        randomSeed = "numeric",

        showLog = "logical",

        hisThreshold = "numeric",
        alignBasis = "logical",
        zeroDimSearch = "logical",
        dimAdjust = "logical",
        dimAdjRatio = "numeric"
    ),
    prototype=prototype(

        maxDim = 1,
        numberOfClusters = 1,
        noiseSize  = 20,
        bestBound  = 1.0,
        errorBound = 0.0001,

        hisSampling = FALSE,
        hisConstSize = 0,
        maxBinPortion = 0.1,

        sampleHeuristic = 3,
        sampleFactor  = 0.003,
        randomSeed = 0,

        showLog = FALSE,

        hisThreshold = 15,
        alignBasis = FALSE,
        zeroDimSearch = FALSE,
        dimAdjust = FALSE,
        dimAdjRatio = 0.99
    )
)

setMethod ("show", "lmclus.params",
function (object)
{
    cat("Linear Manifold Clustering Parameters \n")

    cat("Max dimension:", object@maxDim, "\n")
    cat("Number of clusters:", object@numberOfClusters, "\n")
    cat("Noise size:" , object@noiseSize, "\n")
    cat("Best bound:" , object@bestBound, "\n")
    cat("Error bound:" , object@errorBound , "\n")

    cat("Sample points for distance histogram:" , object@hisSampling , "\n")
    cat("Histogram bins:" , object@hisConstSize , "\n")
    cat("Maximum number of points in a histogram's bin:" , object@maxBinPortion , "\n")

    cat("Sampling heuristic:" , object@sampleHeuristic , "\n")
    cat("Sampling factor:" , object@sampleFactor , "\n")
    cat("Random seed:" , object@randomSeed , "\n")

    cat("Histogram bootstrapping threshold:" , object@hisThreshold , "\n")
    cat("Perform manifold basis alignment:" , object@alignBasis , "\n")
    cat("Perform 0D manifold search:" , object@zeroDimSearch , "\n")

    cat("Manifold dimensionality adjustment:" , object@dimAdjust , "\n")
    cat("Ratio of manifold principal subspace variance:" , object@dimAdjRatio , "\n")

    cat("Show log:" , object@showLog , "\n")
    cat("\n")
})

lmclusPure <- function(X, maxDim, numOfClus, noiseSize, bestBound, errorBound, maxBinPortion,
                       hisSampling, hisConstSize, sampleHeuristic, sampleFactor, randomSeed, showLog,
                       hisThr, algnBasis, zdSearch, dimAdj, dimAdjRatio)
{
    .Call("lmclus", X, maxDim, numOfClus, noiseSize, bestBound, errorBound, maxBinPortion,
          hisSampling, hisConstSize, sampleHeuristic, sampleFactor, randomSeed, showLog,
          hisThr, algnBasis, zdSearch, dimAdj, dimAdjRatio,
          package = "lmclus")
}

kittlerPure <- function(normH, minD, maxD)
{
    .Call("kittler", normH, minD, maxD, package = "lmclus")
}

distToManifoldPure <- function(p, B_T)
{
    .Call("distToManifold", p, B_T, package = "lmclus")
}


lmclus <- function(X, ...) UseMethod("lmclus")

lmclus.default <- function(X, params, ...)
{
    stopifnot(isClass(params), class(params)[1] == "lmclus.params")

    X <- as.matrix(X)

    maxDim <- as.integer(params@maxDim)
    numOfClus <- as.integer(params@numberOfClusters)
    noiseSize <- as.integer(params@noiseSize)
    bestBound <- as.double(params@bestBound)
    errorBound <- as.double(params@errorBound)
    maxBinPortion <- as.double(params@maxBinPortion)
    hisSampling <- as.integer(params@hisSampling)
    hisConstSize <- as.integer(params@hisConstSize)
    sampleHeuristic <- as.integer(params@sampleHeuristic)
    sampleFactor <- as.double(params@sampleFactor)
    randomSeed <- as.integer(params@randomSeed)
    showLog <- as.integer(params@showLog)
    hisThreshold <- as.integer(params@hisThreshold)
    alignBasis <- as.integer(params@alignBasis)
    zeroDimSearch <- as.integer(params@zeroDimSearch)
    dimAdjust <- as.integer(params@dimAdjust)
    dimAdjRatio <- as.double(params@dimAdjRatio)

    res <- lmclusPure(X, maxDim, numOfClus, noiseSize, bestBound, errorBound,
                    maxBinPortion, hisSampling, hisConstSize, sampleHeuristic,
                    sampleFactor, randomSeed, showLog, hisThreshold,
                    alignBasis, zeroDimSearch, dimAdjust, dimAdjRatio)

    res$call <- match.call()

    class(res) <- "lmclus"
    res
}


lmclus.kittler <- function(v, bins)
{
    v <- as.vector(v[!is.na(v)])
    vl <- length(v)
    vmin <- min(v)
    vmax <- max(v)
    if (missing(bins))
        bins <- 100

    h <- hist(v, breaks=bins, plot=FALSE)
    hnorm <- h$counts/vl

    res <- kittlerPure(hnorm, vmin, vmax)

    res <- c(list(hist=hnorm), res)
    return(res)
}

# Calculate distance to cluster
lmclus.distToManifold <- function(p, B, origin)
{
    # Translate to origin if available
    if (missing(origin))
        p <- as.vector(p)
    else
        p <- as.vector(p - origin)

    res <- distToManifoldPure(p, t(B))

    return(res)
}

# Extract specified cluster
lmclus.get_cluster <- function(results, id){
    clust = list(id, results$cluster_dimensions[id], results$origins[[id]],
                 results$bases[[id]], results$clusters[[id]], results$thresholds[id],
                 results$histograms[[id]], results$global_mins[id], results$distances[[id]])
    names(clust) = c("id", "dim", "origin", "basis", "labels",
                        "threshold", "histogram", "global_mins", "distances")
    return(clust)
    print(clust)
}
