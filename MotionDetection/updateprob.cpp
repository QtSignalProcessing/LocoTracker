#include "updateprob.h"

#include "munkres.h"

#include <QDebug>


UpdateProb::UpdateProb(QVector<std::vector<cv::Rect> >&legalBoundRectVec,
                       InitialProb& initProb,QVector< QVector<FeatureExtraction::feature> >& features)
{
    int frameno = legalBoundRectVec.size();
    QVector< float > pL = initProb.getProb();
    bool flag = false;
    int iter = 1;
    cv::Mat visit = cv::Mat::zeros(frameno-1,1,CV_64F);

    while( iter < 20 && flag == false )
    {
        for( int n = 1; n < frameno -1; n++ )
        {
            QVector<FeatureExtraction::feature> sortFeature = features[n];
            std::vector<cv::Rect> sortBbox = legalBoundRectVec[n];
            cv::Mat ltable = initProb.getLtableList()[n];
            cv::Mat sortProb = initProb.getSortProbList()[n];
            if( pL[n] > 0.99 )
            {
                QVector<int> seq = initProb.getSeq()[n];
                visit.at<double>(n) = 1;
                for( int k = -1; k <= 1; k+=2 )
                {
                    if( pL[n+k] >= 0 && pL[n+k] < 1 && seq.size() != 1 ) // no possiblity for pL[n+k] == NaN
                    {
                        visit.at<double>(n+k) = 1;
                        cv::Mat target = cv::Mat::zeros(sortFeature.size(),2,CV_64F);
                        for( int i = 0; i < sortFeature.size(); i++ )
                        {
                            target.at<double>(i,0) = sortFeature[i].distNO;
                            target.at<double>(i,1) = sortFeature[i].xFarest;
                        }
                        cv::Mat source = cv::Mat::zeros(features[n+k].size(),2,CV_64F);
                        for( int i = 0; i < features[n+k].size(); i++ )
                        {
                            source.at<double>(i,0) = features[n+k][i].distNO;
                            source.at<double>(i,1) = features[n+k][i].xFarest;
                        }
                        QVector<int> targetIndice = HungarianLinker(source,target);
                        cv::Mat counter = cv::Mat::zeros(3,1,CV_64F);
                        QVector<int> temseq;
                        for( int j = 0; j < source.rows; j++ )
                        {
                            if( targetIndice[j] > 0 )
                            {
                                qDebug()<<seq.size()<<" "<<targetIndice[j]<<" "<<targetIndice.size();
                                temseq.push_back(seq[targetIndice[j]]);
                                switch (seq[targetIndice[j]])
                                {
                                case 1:
                                    counter.at<double>(0) += 1;
                                    break;
                                case 2:
                                    counter.at<double>(1) += 1;
                                    break;
                                case 3:
                                    counter.at<double>(2) += 1;
                                    break;
                                case 4:
                                    counter.at<double>(1) += 1;
                                    break;
                                case 5:
                                    counter.at<double>(0) += 1;
                                    break;
                                }
                            }
                        }

                        bool inorder = initProb.isUniqueSorted(temseq);
                        bool allpos = true;
                        for( int j = 0; j < source.rows; j++ )
                        {
                            if( targetIndice[j] < 0 )
                            {
                                allpos = false;
                            }
                        }
                        if( allpos && inorder )
                        {
                            for( int j = 0; j < source.rows; j++ )
                            {
                                pL[n+k] = 1;
                                initProb.setLtableList(n+k,j,ltable.row(targetIndice[j]));
                                initProb.setSortProbList(n+k,j,sortProb.row(targetIndice[j]));
                                initProb.setSeq(n+k,j,seq[targetIndice[j]]);
                            }
                        }
                        else if( inorder )
                        {
                            for( int j = 0; j < source.rows; j++ )
                            {
                                if( targetIndice[j] < 0 )
                                {
                                    if( counter.at<double>(0) > 2 || counter.at<double>(1) > 2 || counter.at<double>(2) > 1 )
                                        pL[n+k] = 0;
                                    else if( counter.at<double>(0) == 2 )// not antenna
                                    {
                                        notAntenna(initProb,n,k,j,pL,counter);//update3: 130-172
                                    }
                                    else if( counter.at<double>(1) == 2 ) //not mandible
                                    {
                                        notMandible(initProb,n,k,j,pL,counter);//update3: 174-205
                                    }
                                    else if( counter.at<double>(2) == 1 )
                                    {
                                        notProb(initProb,n,k,j,pL,counter); //update3: 211-251
                                    }
                                }
                                else
                                {
                                    cv::Mat source = initProb.getLtableList()[targetIndice[j]];
                                    initProb.setLtableList(n+k,j,source);
                                    cv::Mat sortProbSouce = initProb.getSortProbList()[targetIndice[j]];
                                    initProb.setSortProbList(n+k,j,sortProbSouce);
                                    initProb.setSeq(n+k,j,seq[targetIndice[j]]);
                                }
                            }
                            inorder = initProb.isUniqueSorted(initProb.getSeq()[n+k]);
                            if( !inorder )
                            {
                                pL[n+k] = 0;
                                for( int j = 0; j < source.rows; j++ )
                                {
                                    if( targetIndice[j] < 0 )
                                        initProb.setSeq(n+k,j,0);
                                }
                            }
                        }
                        else
                        {
                            if( pL[n+k] != 0 )
                                pL[n+k] = 1;
                        }
                    }
                }

                for( int n = 0; n < frameno - 1; n++ )
                {
                    if( pL[n] >= 0 )
                    {
                        if( visit.at<double>(n) == 1 )
                            flag = true;
                        else
                            flag = false;
                    }
                }
            }
        }
        iter++;
    }

    //probLabel{num}{shot}=pL; line 300
    int Winsize = 3;
    for( int n = 3; n < frameno-3; n++ )
    {
        std::vector<cv::Rect> sortBbox = legalBoundRectVec[n];
        if( pL[n] == 0 )
        {
            for( int j = 1; j < Winsize; j++ )
            {
                bool inorder1 = initProb.isUniqueSorted(initProb.getSeq()[n+j]);
                bool inorder2 = initProb.isUniqueSorted(initProb.getSeq()[n-j]);
                int size1 = initProb.getLtableList()[n+j].rows;
                int size2 = initProb.getLtableList()[n].rows;
                int prod = 1;
                int size3 = initProb.getLtableList()[n-j].rows;
                int prod1 = 1;
                for( int i = 0; i < initProb.getSeq()[n-j].size(); i++ )
                {
                    prod *= initProb.getSeq()[n-j][i];
                }
                for( int i = 0; i < initProb.getSeq()[n+j].size(); i++ )
                {
                    prod1 *= initProb.getSeq()[n+j][i];
                }
                if( (size1 == size2) && inorder1 && prod > 0 )
                {
                    initProb.setSeq(n,initProb.getSeq()[n+j]);
                    initProb.setLtableList(n,initProb.getLtableList()[n+j]);
                    break;
                }
                else if( (size3 == size2) && inorder2 && prod1 > 0 )
                {
                    initProb.setSeq(n,initProb.getSeq()[n-j]);
                    initProb.setLtableList(n,initProb.getLtableList()[n-j]);
                    break;
                }
            }
            bool inorder3 = initProb.isUniqueSorted(initProb.getSeq()[n]);
            if( !inorder3 )
            {
                cv::Mat counter = cv::Mat::zeros(3,1,CV_64F);
              //  QVector<int> temseq;
                int L = initProb.getLtableList()[n].rows;
                int L1 = initProb.getSeq()[n].size();
                if( L1 == L )
                {
                    for( int j = 0; j < L; j++ )
                    {
                        int seqj = initProb.getSeq()[n][j];
                        if( seqj > 0 )
                        {
                            switch(seqj)
                            {
                                case 1:
                                if( j== 0 )
                                    counter.at<double>(0) += 1;
                                else
                                    initProb.setSeq(n,j,0);
                                break;
                                case 2:
                                    counter.at<double>(1) += 1;
                                    break;
                                case 3:
                                    counter.at<double>(2) += 1;
                                    break;
                                case 4:
                                    counter.at<double>(1) += 1;
                                    break;
                                case 5:
                                    if( j+1 == L )
                                        counter.at<double>(0) += 1;
                                    else
                                        initProb.setSeq(n,j,0);
                            }
                        }
                    }
                    if( (counter.at<double>(0) ==0) && (counter.at<double>(1)==0) && (counter.at<double>(2)==0) )
                    {
                        switch(L)
                        {
                            case 2:
                            {
                                QVector<int> seq1;
                                seq1.push_back(1);
                                seq1.push_back(5);
                                initProb.setSeq(n,seq1);
                                cv::Mat ltable1 = cv::Mat::zeros(L,3,CV_64F);
                                ltable1.at<double>(0,0) = 1;
                                ltable1.at<double>(1,0) = 1;
                                initProb.setLtableList(n,ltable1);
                                break;
                            }
                            case 3:
                            {
                                QVector<int> seq2;
                                seq2.push_back(1);
                                seq2.push_back(2);
                                seq2.push_back(5);
                                initProb.setSeq(n,seq2);
                                cv::Mat ltable2 = cv::Mat::zeros(L,3,CV_64F);
                                ltable2.at<double>(0,0) = 1;
                                ltable2.at<double>(1,1) = 1;
                                ltable2.at<double>(2,0) = 1;
                                initProb.setLtableList(n,ltable2);
                                break;
                            }
                            case 4:
                            {
                                QVector<int> seq3;
                                seq3.push_back(1);
                                seq3.push_back(2);
                                seq3.push_back(4);
                                seq3.push_back(5);
                                initProb.setSeq(n,seq3);
                                cv::Mat ltable3 = cv::Mat::zeros(L,3,CV_64F);
                                ltable3.at<double>(0,0) = 1;
                                ltable3.at<double>(1,1) = 1;
                                ltable3.at<double>(2,1) = 1;
                                ltable3.at<double>(3,0) = 1;
                                initProb.setLtableList(n,ltable3);
                                break;
                            }
                            case 5:
                            {
                                QVector<int> seq4;
                                seq4.push_back(1);
                                seq4.push_back(2);
                                seq4.push_back(3);
                                seq4.push_back(4);
                                seq4.push_back(5);
                                initProb.setSeq(n,seq4);
                                cv::Mat ltable4 = cv::Mat::zeros(5,L,CV_64F);
                                ltable4.at<double>(0,0) = 1;
                                ltable4.at<double>(1,1) = 1;
                                ltable4.at<double>(2,2) = 1;
                                ltable4.at<double>(3,1) = 1;
                                ltable4.at<double>(4,0) = 1;
                                initProb.setLtableList(n,ltable4);
                                break;
                            }
                        default:
                            qDebug()<<"sth wrong";
                        }
                    }
                    else
                    {
                        cv::Mat counter = cv::Mat::zeros(3,1,CV_64F);
                        for( int k = 0; k < L; k++ )
                        {
                            if( initProb.getSeq()[n][k] > 0 )
                            {
                                int ltablelist1 = initProb.getLtableList()[n].at<double>(k,0);
                                int ltablelist2 = initProb.getLtableList()[n].at<double>(k,1);
                                int ltablelist3 = initProb.getLtableList()[n].at<double>(k,2);
                                if( (ltablelist1 == 1) && (ltablelist2 == 0) && (ltablelist3 == 0) )
                                {
                                    if( counter.at<double>(0) == 0 && k == 0 )
                                    {
                                        initProb.setSeq(n,k,1);
                                        counter.at<double>(0) = 1;
                                    }
                                    else
                                    {
                                        initProb.setSeq(n,k,5);
                                        counter.at<double>(0) = 5;
                                    }
                                }
                                else if( ltablelist1 == 0 && ltablelist2 == 1 && ltablelist3 == 0 )
                                {
                                    if( counter.at<double>(1) == 0 )
                                    {
                                        initProb.setSeq(n,k,2);
                                        counter.at<double>(1) = 1;
                                    }
                                    else
                                    {
                                        initProb.setSeq(n,k,4);
                                        counter.at<double>(1) = 2;
                                    }
                                }
                                else
                                {
                                    initProb.setSeq(n,k,3);
                                    counter.at<double>(2) = 1;
                                }
                            }
                        }
                        QVector<int> seqn = initProb.getSeq()[n];
                        for( int k = 0; k < std::floor((float)L/2); k++ )
                        {
                            if( seqn[k] == 0 )
                            {
                                if( k == 0 )
                                {
                                    if( seqn[1] != 0 )
                                        initProb.setSeq(n,k,seqn[k+1]-1);
                                    else
                                        initProb.setSeq(n,k,1);
                                }
                                else
                                {
                                    initProb.setSeq(n,k,seqn[k-1]-1);
                                }
                                switch ( seqn[k] )
                                {
                                    case 1:
                                    {
                                        cv::Mat tmp = cv::Mat::zeros(1,3,CV_64F);
                                        tmp.at<double>(0) = 1;
                                        initProb.setLtableList(n,k,tmp);
                                        break;
                                    }
                                    case 2:
                                    {
                                        cv::Mat tmp = cv::Mat::zeros(1,3,CV_64F);
                                        tmp.at<double>(1) = 1;
                                        initProb.setLtableList(n,k,tmp);
                                        break;
                                    }
                                    case 3:
                                    {
                                        cv::Mat tmp = cv::Mat::zeros(1,3,CV_64F);
                                        tmp.at<double>(2) = 1;
                                        initProb.setLtableList(n,k,tmp);
                                        break;
                                    }
                                    case 4:
                                    {
                                        cv::Mat tmp = cv::Mat::zeros(1,3,CV_64F);
                                        tmp.at<double>(1) = 1;
                                        initProb.setLtableList(n,k,tmp);
                                        break;
                                    }
                                    case 5:
                                    {
                                        cv::Mat tmp = cv::Mat::zeros(1,3,CV_64F);
                                        tmp.at<double>(0) = 1;
                                        initProb.setLtableList(n,k,tmp);
                                        break;
                                    }
                                }
                            }
                        }
                        for( int k = L - 1; k >= std::floor((float)L/2)+1; k-- )
                        {
                            QVector<int> seqn = initProb.getSeq()[n];
                            if( k == 0 )
                            {
                                if( seqn[1] != 0 )
                                    initProb.setSeq(n,k,seqn[k+1]);
                                else
                                    initProb.setSeq(n,k,1);
                            }
                            else
                                initProb.setSeq(n,k,seqn[k-1]+1);
                            switch ( seqn[k] )
                            {
                                case 1:
                                {
                                    cv::Mat tmp = cv::Mat::zeros(1,3,CV_64F);
                                    tmp.at<double>(0) = 1;
                                    initProb.setLtableList(n,k,tmp);
                                    break;
                                }
                                case 2:
                                {
                                    cv::Mat tmp = cv::Mat::zeros(1,3,CV_64F);
                                    tmp.at<double>(1) = 1;
                                    initProb.setLtableList(n,k,tmp);
                                    break;
                                }
                                case 3:
                                {
                                    cv::Mat tmp = cv::Mat::zeros(1,3,CV_64F);
                                    tmp.at<double>(2) = 1;
                                    initProb.setLtableList(n,k,tmp);
                                    break;
                                }
                                case 4:
                                {
                                    cv::Mat tmp = cv::Mat::zeros(1,3,CV_64F);
                                    tmp.at<double>(1) = 1;
                                    initProb.setLtableList(n,k,tmp);
                                    break;
                                }
                                case 5:
                                {
                                    cv::Mat tmp = cv::Mat::zeros(1,3,CV_64F);
                                    tmp.at<double>(0) = 1;
                                    initProb.setLtableList(n,k,tmp);
                                    break;
                                }
                            }
                        }

                    }
                    for( int j = 0; j < L; j++ )
                    {
                        QVector<int> seqn = initProb.getSeq()[n];
                        if( seqn[j] ==2 || seqn[j] == 3 || seqn[j] == 4 )
                            if( sortBbox[j].x > initProb.getCentroid().x )//TODO
                                initProb.setSeq(n,j,0);
                    }
                }
            }
        }
    }
    //Displaybee


}

UpdateProb::UpdateProb(){}

void UpdateProb::firstForLoop()
{

}

QVector<int> UpdateProb::HungarianLinker(const cv::Mat &source, const cv::Mat &target)
{
    int numOFSourcePt = source.rows;
    int numOfTargetPt = target.rows;
    cv::Mat D = cv::Mat::ones(numOFSourcePt,numOfTargetPt,CV_64F);
    for( int i = 0; i < numOFSourcePt; i++ )
    {
        cv::Mat currentPt(numOfTargetPt,source.cols,CV_64F);
        for( int j = 0; j < numOfTargetPt; j++ )
            source.row(i).copyTo(currentPt.row(j));
        cv::Mat diff_coords = target - currentPt;

     //   diff_coords = diff_coords * diff_coords;
        for(int j = 0; j < diff_coords.rows; j++ )
        {
            for( int k = 0; k < diff_coords.cols; k++ )
            {
                diff_coords.at<double>(j,k) *= diff_coords.at<double>(j,k);
            }
        }
        cv::Mat squareDist=cv::Mat::zeros(1,numOfTargetPt,CV_64F);
        for( int j = 0; j < numOfTargetPt; j++ )
        {
            squareDist.at<double>(0,j) = cv::sum(diff_coords.row(j))[0];
        }
        squareDist.copyTo(D.row(i));
    }
    Munkres m;
   // m.diag(true);
    m.solve(D);
   return m.getOptVec();
}

inline void UpdateProb::updateSortProbList(InitialProb &initProb, int n, int k, int j, int col1, int col2,int col3,double sort1,double sort2) // col1 = 0, col2 = 1, col3 = 2
{
    initProb.setSortProbList(n+k,j,col3,0);
    initProb.setSortProbList(n+k,j,col1,sort1/(sort1+sort2));
    sort1 = initProb.getSortProbList()[n+k].at<double>(j,col1);
    initProb.setSortProbList(n+k,j,col2,1-sort1);
}

inline void UpdateProb::updateLtableList(InitialProb &initProb, int indexOfList,int row,int indOf1)
{
    for( int i = 0; i < 3; i++ )
    {
        if( i != indOf1 )
            initProb.setLtableList(indexOfList,row,i,0);
        else
            initProb.setLtableList(indexOfList,row,i,1);
    }
}

void UpdateProb::notAntenna(InitialProb &initProb, int n, int k, int j,QVector<float>& pL,cv::Mat& counter)
{
    double tmp1 = initProb.getSortProbList()[n+k].at<double>(j,1);
    double tmp2 = initProb.getSortProbList()[n+k].at<double>(j,2);
    if( tmp1 + tmp2 != 0 )
    {
        updateSortProbList(initProb,n,k,j,1,2,0,tmp1,tmp2);
    }
    else
    {
        initProb.setLtableList(n+k,j,0,0);
        double containMandible = initProb.getSortFeatureList()[n+k].at<double>(j,5);
        double edgeCounter1 = initProb.getSortFeatureList()[n+k].at<double>(j,6);
        if( containMandible > 0 && edgeCounter1 ==1 )
        {
            initProb.setSortProbList(n+k,j,1,0);
            initProb.setSortProbList(n+k,j,2,1);
        }
        else
        {
            initProb.setSortProbList(n+k,j,1,0.5);
            initProb.setSortProbList(n+k,j,2,0.5);
        }
        tmp1 = initProb.getSortProbList()[n+k].at<double>(j,1);
        tmp2 = initProb.getSortProbList()[n+k].at<double>(j,2);

        if( tmp1 < tmp2 )
        {
            updateLtableList(initProb,n+k,j,2);
            initProb.setSeq(n+k,j,3);
            pL[n+k] = initProb.getSortProbList()[n+k].at<double>(j,3);
        }
        else if( tmp1 > tmp2 )
        {
            updateLtableList(initProb,n+k,j,1);
            pL[n+k] = initProb.getSortProbList()[n+k].at<double>(j,1);
            if( counter.at<double>(1) == 0 )
                initProb.setSeq(n+k,j,2);
            else
                initProb.setSeq(n+k,j,4);
        }
    }
}

void UpdateProb::notMandible(InitialProb &initProb, int n, int k, int j, QVector<float> &pL, cv::Mat &counter)
{
    double sort1 = initProb.getSortProbList()[n+k].at<double>(j,0);
    double sort3 = initProb.getSortProbList()[n+k].at<double>(j,2);
    if( sort1 + sort3 != 0 )
    {
        updateSortProbList(initProb,n,k,j,0,2,1,sort1,sort3);
    }
    else
    {
        initProb.setSortProbList(n+k,j,0,0);
        initProb.setSortProbList(n+k,j,1,0.5);
        initProb.setSortProbList(n+k,j,2,0.5);
    }
    sort1 = initProb.getSortProbList()[n+k].at<double>(j,0);
    sort3 = initProb.getSortProbList()[n+k].at<double>(j,2);
    if( sort1 < sort3 )
    {
        updateLtableList(initProb,n+k,j,2);
        initProb.setSeq(n+k,j,3);
        pL[n+k] = initProb.getSortProbList()[n+k].at<double>(j,2);
    }
    else if( sort1 > sort3 )
    {
        updateLtableList(initProb,n+k,j,0);
        if( counter.at<double>(0) == 0 )
            initProb.setSeq(n+k,j,1);
        else
            initProb.setSeq(n+k,j,5);
        pL[n+k] = initProb.getSortProbList()[n+k].at<double>(j,0);
    }
}

void UpdateProb::notProb(InitialProb &initProb, int n, int k, int j, QVector<float> &pL, cv::Mat &counter)
{
    double sort1 = initProb.getSortProbList()[n+k].at<double>(j,0);
    double sort2 = initProb.getSortProbList()[n+k].at<double>(j,1);
    if( sort1 + sort2 != 0 )
    {
        updateSortProbList(initProb,n,k,j,0,1,2,sort1,sort2);
    }
    else
    {
        initProb.setLtableList(n+k,j,2,0);
        double sortF6 = initProb.getSortFeatureList()[n+k].at<double>(j,5);
        if( sortF6 > 0 )
        {
            initProb.setSortProbList(n+k,j,0,1);
            initProb.setSortProbList(n+k,j,1,0);
        }
        initProb.setSortProbList(n+k,j,0,0.5);
        initProb.setSortProbList(n+k,j,1,0.5);
    }
    sort1 = initProb.getSortProbList()[n+k].at<double>(j,0);
    sort2 = initProb.getSortProbList()[n+k].at<double>(j,1);
    if( sort1 < sort2 )
    {
        updateLtableList(initProb,n+k,j,1);
        if( counter.at<double>(1) == 0 )
            initProb.setSeq(n+k,j,2);
        else
            initProb.setSeq(n+k,j,4);
        pL[n+k] = initProb.getSortProbList()[n+k].at<double>(j,1);
    }
    else if( sort1 > sort2 )
    {
        updateLtableList(initProb,n+k,j,0);
        if( counter.at<double>(0) == 0 )
            initProb.setSeq(n+k,j,1);
        else
            initProb.setSeq(n+k,j,5);
        pL[n+k] = initProb.getSortProbList()[n+k].at<double>(j,0);
    }
}
