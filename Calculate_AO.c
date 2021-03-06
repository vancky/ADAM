#include"utils.h"
#include"time.h"
#include"globals.h"
void dhapke_bright_fac(double *E,double *E0,double mu,double mu0,double *p,double th,double *rss,double *rdssdmu,double *rdssdmu0);
void dec_vector(double x[3],double y[3],double z[3])
{
  z[0]=x[0]-y[0];
  z[1]=x[1]-y[1];
  z[2]=x[2]-y[2];
}

double Calculate_AO(int *tlist,double *vlist,int nfac,int nvert,double *angles,double *Eo,double *E0o,double *up,double TIME,double dist,double *freqx,double *freqy,int nfreq,double *offset,double *Fr,double *Fi,double *A,double *Alimit)
{
/*
 * Possibility of setting albedos of AO facets in global variable INI_SET_AO_ALBEDO
 * If the variable is not null, values are used only in Calculate_AO, NOT in Calculate_AO_deriv
 */
 double complex *F0;
// double complex *dFda,*dFdb,*dFdc,*dFdd,*dFdh,*dFdg;
 F0=calloc(nfreq,sizeof(double complex));
 double AOalb=1;
 double dt1,dt2;
 double complex *F=calloc(nfreq,sizeof(double complex));
 double M[3][3],dMb[3][3],dMo[3][3],dMl[3][3],Mt[3][3];
 double R[3][3],Rdb[3][3],Rdl[3][3],Rdo[3][3],RT[3][3];
 double E[3],E0[3];
 double normalr[3],side1[3],side2[3];
 double dechdx[3],dechdy[3],dechdz[3],dechdA[3];
 double dAlb3[3];
 double n[3],*nb,*cent;
 double *vb1,*vb2,*vb3;
 double vr1[3],vr2[3],vr3[3];
 double *v1,*v2,*v3;
 double scale;
 double complex tscale,FTC;
 double dp;
 double B,TB=0;
 double norm;
 int t1,t2,t3,blocker,sign;
int j1,j2,j3;
 double mu,mu0,area,mub,ech,rexp;
 double *normal,*centroid;
 int *visible;
 int tb1,tb2,tb3; //Indices to the vertices of possible blocker facet
 int blocked=0;
 //Stuff relating to albedos
  double la=0;
 double ha=1; /*These are the albedo limits*/
 double alb_term=0;
 double alb=0;
 int albedo=0;
 if(A!=NULL && INI_IGNORE_AO_ALBEDO==0)
 {
     albedo=1;
     la=Alimit[0];
     ha=Alimit[1];
 }
 //Distance km->arcsec
 dp=1/(dist*149597871.0)*180.0/PI*3600.0;
 visible=calloc(nfac,sizeof(int));
  //Allocate for memory
// normal=(double*)malloc(3*nfac*sizeof(double));
 // centroid=(double*)mxCalloc(3*nfac,sizeof(double));
 // IndexofBlocks=(int*)mxCalloc(nfac,sizeof(int));
// NumofBlocks=(int*)mxCalloc(nfac,sizeof(int));
  //Calculate frame change matrix
  Calculate_Frame_Matrix(Eo,up,R);
 
 //FacetsOverHorizon(tlist,vlist,nfac,nvert,normal,centroid,NumofBlocks,IndexofBlocks);
 
 rotate(angles[0],angles[1],angles[2],angles[3],TIME,M,dMb,dMl,dMo);
 //Construct asteroid->Camera frame matrix, which is
 //asteroid->world frame->camera frame
 transpose(M,Mt); //Transpose, since we rotate the model, not view directions
 mult_mat(R,Mt,RT); 
 mult_vector(M,Eo,E);
 mult_vector(M,E0o,E0);

 /*For each facet,
  * 1)Check if facet is visible
  * 2) Calculate echo
  * 3) Convert triangle to range-Doppler frame
  * 4) Calculate FT
  */
//Find actual blockers
FindActualBlockers(tlist,vlist,nfac,nvert,E,E0,1,visible);
 //visible is nfac vector, visible[j]=1 if facet (j+1)th facet is visible
//NOTE INDEXING
//mexPrintf("%f %f %f\n",vlist[0],vlist[1],vlist[2]);
 for(int j=0;j<nfac;j++)
 {
   if(visible[j]==0)
     continue;
  //Calculate normal from facet vertices
   //Vertex indices of the current facet
   //Note that C indices from 0, matlab from 1
   if(INI_SET_AO_ALBEDO!=NULL)
       AOalb=INI_SET_AO_ALBEDO[j];
   j1=tlist[j*3]-1;
   j2=tlist[j*3+1]-1;
   j3=tlist[j*3+2]-1;
   //Current vertices
   
   v1=vlist+j1*3;
   v2=vlist+j2*3;
   v3=vlist+j3*3;
   
   
   //Calculate normals and centroids
   for(int i=0;i<3;i++)
   {
     
     side1[i]=dp*(v2[i]-v1[i]); //Convert km->arcsec
     side2[i]=dp*(v3[i]-v1[i]);
    
   }
   cross(side1,side2,n);
   norm=NORM(n);
   n[0]=n[0]/norm;
   n[1]=n[1]/norm;
   n[2]=n[2]/norm;
   
   mu=DOT(E,n);
   mu0=DOT(E0,n);
  //Convert to camera frame
   mult_vector(RT,v1,vr1);
   mult_vector(RT,v2,vr2);
   mult_vector(RT,v3,vr3);
   for(int i=0;i<3;i++)
   {
     vr1[i]=dp*vr1[i];
     vr2[i]=dp*vr2[i];
     vr3[i]=dp*vr3[i];
   }
    
     //Now we should convert to frequency domain, ie calculate the contribution of each facet
     Calc_FTC(freqx,freqy,nfreq,vr1[0],vr1[1],vr2[0],vr2[1],vr3[0],vr3[1],F0);
     //printf("Input to calc_ftc: freq: %f %f vr: %f %f %f %f %f %f\n",freqx[1],freqy[1],vr1[0],vr1[1],vr2[0],vr2[1],vr3[0],vr3[1]);
     //printf("F0: %f %f\n",creal(F0[1]),cimag(F0[1]));
     //Note that we sum to F at each round, does not work, we need to multiply with the echo
     //Derivatives wrt angles
   area=0.5*norm;

 
    //Albedo stuff: 
   if(albedo==0)
        {
            alb=0;
            alb_term=1;
        }
        else
        {
            //alb=A[j];
            
            alb_term=Albedo_Term(tlist,vlist,nfac,nvert,Alimit,A,j,dAlb3);
        }
    if(INI_HAPKE!=NULL)
        dhapke_bright_fac(E,E0,mu,mu0,INI_HAPKE,INI_HAPKE[4],&B,&dt2,&dt1);
    else
        B=mu0*(1.0/(mu+mu0)+0.1); //mu removed here
     for(int jf=0;jf<nfreq;jf++)
     {
      
       F[jf]+=alb_term*AOalb*B*F0[jf];
      
       
      }
      if(INI_HAPKE!=NULL)
          TB=TB+alb_term*AOalb*B*mu*area;
      else
        TB=TB+alb_term*AOalb*B*area*mu;
    

}

for(int j=0;j<nfreq;j++)
{
 //   printf("j: %d, F[j] real: %f, F[j] imag: %f\n",j,creal(F[j]),cimag(F[j]));
  Fr[j]=creal(cexp(2.0*PI*I*(offset[0]*freqx[j]+offset[1]*freqy[j]))*F[j]/TB);
  Fi[j]=cimag(cexp(2.0*PI*I*(offset[0]*freqx[j]+offset[1]*freqy[j]))*F[j]/TB);
  
}
free(visible);
free(F0);
free(F);
return TB;
}

void Calculate_AO_deriv(int *tlist,double *vlist,int nfac,int nvert,double *angles,double *Eo,double *E0o,double *up,double TIME,double dist,double *freqx,double *freqy,int nfreq,double *offset,double *Fr,double *Fi,double *dFdxr,double *dFdxi,double *dFdyr,double *dFdyi,double *dFdzr,double *dFdzi,double *dFdAr,double *dFdAi,double *dFdoffr,double *dFdoffi,double *A,double *Alimit,double *dAr,double *dAi)
{
 // clock_t start=clock(),diff;  
  double complex *F=calloc(nfreq,sizeof(double complex));
 double complex *F0,*FTda,*FTdb,*FTdc,*FTdd,*FTdh,*FTdg,*FTdx,*FTdy,*FTdz,*FTdA;
 FTdx=(double complex*)calloc(nfreq*nvert,sizeof(double complex));
 FTdy=(double complex*)calloc(nfreq*nvert,sizeof(double complex));
 FTdz=(double complex*)calloc(nfreq*nvert,sizeof(double complex));
 FTdA=(double complex*)calloc(nfreq*3,sizeof(double complex));
 F0=(double complex*)calloc(nfreq,sizeof(double complex));
 FTda=(double complex*)calloc(nfreq,sizeof(double complex));
 FTdb=(double complex*)calloc(nfreq,sizeof(double complex));
 FTdc=(double complex*)calloc(nfreq,sizeof(double complex));
 FTdd=(double complex*)calloc(nfreq,sizeof(double complex));
 FTdh=(double complex*)calloc(nfreq,sizeof(double complex));
 FTdg=(double complex*)calloc(nfreq,sizeof(double complex));
 
 double M[3][3],dMb[3][3],dMo[3][3],dMl[3][3],Mt[3][3],dMbT[3][3],dMoT[3][3],dMlT[3][3]; //Rotation matrices and their derivatives and transposes
 double R[3][3],Rdb[3][3],Rdl[3][3],Rdo[3][3],RT[3][3]; //Projection matrix, and derivatives of combined projection+rotation matrix
 double dEdb[3],dEdl[3],dEdo[3],dE0db[3],dE0dl[3],dE0do[3];
 double dndx1[3],dndx2[3],dndx3[3],dndy1[3],dndy2[3],dndy3[3],dndz1[3],dndz2[3],dndz3[3]; //Derivatives of the facet normal vector
 double dBdx1,dBdy1,dBdz1,dBdx2,dBdy2,dBdz2,dBdx3,dBdy3,dBdz3,dBdb,dBdl,dBdo; //Derivatives of facet brightness
 double *dTBdx,*dTBdy,*dTBdz; //Derivatives of total brightness, allocating memory
 double dTBdA[3]={0.0,0.0,0.0};
 double dAlb3[3];
 dTBdx=calloc(nvert,sizeof(double));
 dTBdy=calloc(nvert,sizeof(double));
 dTBdz=calloc(nvert,sizeof(double));
 double dmudx1,dmudy1,dmudz1,dmudx2,dmudy2,dmudz2,dmudx3,dmudy3,dmudz3;
 double dmu0dx1,dmu0dy1,dmu0dz1,dmu0dx2,dmu0dy2,dmu0dz2,dmu0dx3,dmu0dy3,dmu0dz3;
 double dmudl,dmudb,dmudo,dmu0dl,dmu0db,dmu0do; //Derivatives of mu and mu0
 double dAdx[3],dAdy[3],dAdz[3]; //Facet area derivatives
 double dadx,dady,dadz,dbdx,dbdy,dbdz; //Derivatives of projected vertices
 double E[3],E0[3]; //Earth and Sun direction, rotated
 double side1[3],side2[3];
 double n[3];
 double v1db[3],v2db[3],v3db[3],v1dl[3],v2dl[3],v3dl[3],v1do[3],v2do[3],v3do[3]; //Derivatives of 2d vertices wrt angles
 double vr1[3],vr2[3],vr3[3];
 double v1[3],v2[3],v3[3];
 double complex scale;
//Albedo Stuff
 double alb=0;
  double *dSdalb=NULL;
  double *dAlb=NULL;
  double *dAlbTB=NULL;
  int albedo=0;
  double la,ha,alb_term=0;
  double albexp=0;
 if( A!=NULL && INI_IGNORE_AO_ALBEDO==0)
 {
     albedo=1;
     
     la=Alimit[0];
     ha=Alimit[1];
     if(INI_FIT_AO_ALBEDO==1)
     {
     dAlb=calloc(nfreq*nvert,sizeof(double complex));
     dAlbTB=calloc(nvert,sizeof(double));
     }
 }
 
 
 double dp;
 double B,TB=0.0;
 double norm;
 double mut,mu0t;
 int t1,t2,t3;
int j1,j2,j3;
 double mu,mu0,area;
 double *normal;
 int *visible;
 int tb1,tb2,tb3; //Indices to the vertices of possible blocker facet
 int blocked=0;
 //Distance km->arcsec
 dp=1/(dist*149597871.0)*180.0/PI*3600.0;
 visible=(int*)calloc(nfac,sizeof(int));
 //Calculate_Frame_Matrix_Derivatives(Eo,angles,TIME,rfreq,R,Rdb,Rdl,Rdo);
  Calculate_Frame_Matrix(Eo,up,R);
  
  
  //Calculate frame change matrix
 
 
 //FacetsOverHorizon(tlist,vlist,nfac,nvert,normal,centroid,NumofBlocks,IndexofBlocks);
 
 rotate(angles[0],angles[1],angles[2],angles[3],TIME,M,dMb,dMl,dMo);
 //Construct asteroid->Camera frame matrix, which is
 //asteroid->world frame->camera frame
 transpose(M,Mt); //Transpose, since we rotate the model, not view directions
 transpose(dMb,dMbT);
 transpose(dMl,dMlT);
 transpose(dMo,dMoT);
 mult_mat(R,Mt,RT); 
 mult_vector(M,Eo,E);
 mult_vector(M,E0o,E0);
 
 mult_mat(R,dMbT,Rdb);
 mult_mat(R,dMlT,Rdl);
 mult_mat(R,dMoT,Rdo);
//Derivatives of E,E0 wrt beta,lambda,omega
 mult_vector(dMb,Eo,dEdb);
 mult_vector(dMl,Eo,dEdl);
 mult_vector(dMo,Eo,dEdo);
 mult_vector(dMb,E0o,dE0db);
 mult_vector(dMl,E0o,dE0dl);
 mult_vector(dMo,E0o,dE0do);
 dadx=RT[0][0];
  dady=RT[0][1];
  dadz=RT[0][2];
  dbdx=RT[1][0];
  dbdy=RT[1][1];
  dbdz=RT[1][2];
 /*For each facet,
  * 1)Check if facet is visible
  * 2) Calculate echo
  * 3) Convert triangle to range-Doppler frame
  * 4) Calculate FT
  */
//Find actual blockers
FindActualBlockers(tlist,vlist,nfac,nvert,E,E0,1,visible);
 //visible is nfac vector, visible[j]=1 if facet (j+1)th facet is visible
//NOTE INDEXING


 //for(int j=0;j<nfac;j++)
for(int j=0;j<nfac;j++) 
{
   if(visible[j]==0)
     continue;
  //Calculate normal from facet vertices
   //Vertex indices of the current facet
   //Note that C indices from 0, matlab from 1
   j1=tlist[j*3]-1;
   j2=tlist[j*3+1]-1;
   j3=tlist[j*3+2]-1;
   //Current vertices
  
   
   for(int i=0;i<3;i++)
   {
   v1[i]=*(vlist+j1*3+i)*dp; //convert km->arcsec
   v2[i]=*(vlist+j2*3+i)*dp;
   v3[i]=*(vlist+j3*3+i)*dp;
   }
   //Albedo Stuff
   if(albedo==0)
        {
            alb=0;
            alb_term=1;
        }
        else
        {
           // alb=A[j];
           // dSdalb[e*numfac+j]=phase*(ha-la)*exp(alb)/pow(exp(alb)+1,2)*Scatt*area[j];
            alb_term=Albedo_Term(tlist,vlist,nfac,nvert,Alimit,A,j,dAlb3);
            
        }
    //
    //Calculate Normal derivatives (in the original frame)
    Calculate_Area_and_Normal_Derivative(v1,v2,v3,n,dndx1,dndx2,dndx3,dndy1,dndy2,dndy3,dndz1,dndz2,dndz3,&area,dAdx,dAdy,dAdz);
    
   //Calculate normals and centroids
   
   mu=DOT(E,n);
   mu0=DOT(E0,n);
  //Convert to camera frame
   mult_vector(RT,v1,vr1);
   mult_vector(RT,v2,vr2);
   mult_vector(RT,v3,vr3);
  
    
     //Now we should convert to frequency domain, ie calculate the contribution of each facet
  //   Calc_FTC(freqx,freqy,nfreq,vr1[0],vr1[1],vr2[0],vr2[1],vr3[0],vr3[1],F0);
     Calc_FTC_deriv(freqx,freqy,nfreq,vr1[0],vr1[1],vr2[0],vr2[1],vr3[0],vr3[1],F0,FTda,FTdb,FTdc,FTdd,FTdg,FTdh);
  
    //Derivatives wrt angles
     mult_vector(Rdb,v1,v1db);
     mult_vector(Rdb,v2,v2db);
     mult_vector(Rdb,v3,v3db);
     mult_vector(Rdl,v1,v1dl);
     mult_vector(Rdl,v2,v2dl);
     mult_vector(Rdl,v3,v3dl);
     mult_vector(Rdo,v1,v1do);
     mult_vector(Rdo,v2,v2do);
     mult_vector(Rdo,v3,v3do);
     //Derivatives of mu,mu0
     dmudx1=DOT(E,dndx1);
     dmudx2=DOT(E,dndx2);
     dmudx3=DOT(E,dndx3);
     dmudy1=DOT(E,dndy1);
     dmudy2=DOT(E,dndy2);
     dmudy3=DOT(E,dndy3);
     dmudz1=DOT(E,dndz1);
     dmudz2=DOT(E,dndz2);
     dmudz3=DOT(E,dndz3);
     dmudb=DOT(dEdb,n);
     dmudl=DOT(dEdl,n);
     dmudo=DOT(dEdo,n);
     dmu0dx1=DOT(E0,dndx1);
     dmu0dx2=DOT(E0,dndx2);
     dmu0dx3=DOT(E0,dndx3);
     dmu0dy1=DOT(E0,dndy1);
     dmu0dy2=DOT(E0,dndy2);
     dmu0dy3=DOT(E0,dndy3);
     dmu0dz1=DOT(E0,dndz1);
     dmu0dz2=DOT(E0,dndz2);
     dmu0dz3=DOT(E0,dndz3);
     dmu0db=DOT(dE0db,n);
     dmu0dl=DOT(dE0dl,n);
     dmu0do=DOT(dE0do,n);
   if(INI_HAPKE!=NULL)
        dhapke_bright_fac(E,E0,mu,mu0,INI_HAPKE,INI_HAPKE[4],&B,&mut,&mu0t);
    else
    {
        B=mu0*(1.0/(mu+mu0)+0.1); //mu removed here
        mu0t=(mu/pow(mu+mu0,2)+0.1);
        mut=-mu0/pow(mu+mu0,2);
    }
   //Derivatives of B
   
   dBdx1=mu0t*dmu0dx1+mut*dmudx1;
   dBdx2=mu0t*dmu0dx2+mut*dmudx2;
   dBdx3=mu0t*dmu0dx3+mut*dmudx3;
   
   dBdy1=mu0t*dmu0dy1+mut*dmudy1;
   dBdy2=mu0t*dmu0dy2+mut*dmudy2;
   dBdy3=mu0t*dmu0dy3+mut*dmudy3;
   
   dBdz1=mu0t*dmu0dz1+mut*dmudz1;
   dBdz2=mu0t*dmu0dz2+mut*dmudz2;
   dBdz3=mu0t*dmu0dz3+mut*dmudz3;
   dBdb=mu0t*dmu0db+mut*dmudb;
   dBdl=mu0t*dmu0dl+mut*dmudl;
   dBdo=mu0t*dmu0do+mut*dmudo;
   //Derivative of total brightness
   
       dTBdx[j1]+=dBdx1*area*mu+B*dAdx[0]*mu+B*area*dmudx1;
       dTBdx[j2]+=dBdx2*area*mu+B*dAdx[1]*mu+B*area*dmudx2;
       dTBdx[j3]+=dBdx3*area*mu+B*dAdx[2]*mu+B*area*dmudx3;
       dTBdy[j1]+=dBdy1*area*mu+B*dAdy[0]*mu+B*area*dmudy1;
       dTBdy[j2]+=dBdy2*area*mu+B*dAdy[1]*mu+B*area*dmudy2;
       dTBdy[j3]+=dBdy3*area*mu+B*dAdy[2]*mu+B*area*dmudy3;
       dTBdz[j1]+=dBdz1*area*mu+B*dAdz[0]*mu+B*area*dmudz1;
       dTBdz[j2]+=dBdz2*area*mu+B*dAdz[1]*mu+B*area*dmudz2;
       dTBdz[j3]+=dBdz3*area*mu+B*dAdz[2]*mu+B*area*dmudz3;
       dTBdA[0]+=dBdb*area*mu+B*area*dmudb;
       dTBdA[1]+=dBdl*area*mu+B*area*dmudl;
       dTBdA[2]+=dBdo*area*mu+B*area*dmudo;
       
   
   for(int jf=0;jf<nfreq;jf++)
     {
     F[jf]+=alb_term*B*F0[jf];
     if(albedo==1 && INI_FIT_AO_ALBEDO==1)
     {
        dAlb[jf*nvert+j1]+=dAlb3[0]*B*F0[jf];
        dAlb[jf*nvert+j2]+=dAlb3[1]*B*F0[jf];
        dAlb[jf*nvert+j3]+=dAlb3[2]*B*F0[jf];
        
     }
       FTdx[jf*nvert+j1]+=dBdx1*F0[jf]+B*(FTda[jf]*dadx+FTdb[jf]*dbdx);
       FTdx[jf*nvert+j2]+=dBdx2*F0[jf]+B*(FTdc[jf]*dadx+FTdd[jf]*dbdx);
       FTdx[jf*nvert+j3]+=dBdx3*F0[jf]+B*(FTdg[jf]*dadx+FTdh[jf]*dbdx);
       
       FTdy[jf*nvert+j1]+=dBdy1*F0[jf]+B*(FTda[jf]*dady+FTdb[jf]*dbdy);
       FTdy[jf*nvert+j2]+=dBdy2*F0[jf]+B*(FTdc[jf]*dady+FTdd[jf]*dbdy);
       FTdy[jf*nvert+j3]+=dBdy3*F0[jf]+B*(FTdg[jf]*dady+FTdh[jf]*dbdy);
       
       FTdz[jf*nvert+j1]+=dBdz1*F0[jf]+B*(FTda[jf]*dadz+FTdb[jf]*dbdz);
       FTdz[jf*nvert+j2]+=dBdz2*F0[jf]+B*(FTdc[jf]*dadz+FTdd[jf]*dbdz);
       FTdz[jf*nvert+j3]+=dBdz3*F0[jf]+B*(FTdg[jf]*dadz+FTdh[jf]*dbdz);
        //angle derivatives
       
       FTdA[jf*3+0]+=dBdb*F0[jf]+B*(FTda[jf]*v1db[0]+FTdb[jf]*v1db[1]+FTdc[jf]*v2db[0]+FTdd[jf]*v2db[1]+FTdg[jf]*v3db[0]+FTdh[jf]*v3db[1]);
       FTdA[jf*3+1]+=dBdl*F0[jf]+B*(FTda[jf]*v1dl[0]+FTdb[jf]*v1dl[1]+FTdc[jf]*v2dl[0]+FTdd[jf]*v2dl[1]+FTdg[jf]*v3dl[0]+FTdh[jf]*v3dl[1]);
       FTdA[jf*3+2]+=dBdo*F0[jf]+B*(FTda[jf]*v1do[0]+FTdb[jf]*v1do[1]+FTdc[jf]*v2do[0]+FTdd[jf]*v2do[1]+FTdg[jf]*v3do[0]+FTdh[jf]*v3do[1]);
     }
    if(INI_HAPKE!=NULL)
    {
        TB=TB+alb_term*B*area*mu;
        if(albedo==1 && INI_FIT_AO_ALBEDO==1)
        {
            
            dAlbTB[j1]+=dAlb3[0]*B*area*mu;
            dAlbTB[j2]+=dAlb3[1]*B*area*mu;
            dAlbTB[j3]+=dAlb3[2]*B*area*mu;
        }
    }
    else
    {
      TB=TB+alb_term*B*area*mu;
        if(albedo==1 && INI_FIT_AO_ALBEDO==1)
        {
            
           dAlbTB[j1]+=dAlb3[0]*B*area*mu;
            dAlbTB[j2]+=dAlb3[1]*B*area*mu;
            dAlbTB[j3]+=dAlb3[2]*B*area*mu;
        }
    }       
}
//mexPrintf("Total brightness: %f\n",TB);  
//Normalize with total brightness
double TB2=pow(TB,2);
double complex temp;
for(int j=0;j<nfreq;j++)
{
  scale=cexp(2.0*PI*I*(offset[0]*freqx[j]+offset[1]*freqy[j]));
  for(int k=0;k<nvert;k++)
  {
    dFdxr[j*nvert+k]=creal(dp*scale*(FTdx[j*nvert+k]*TB-F[j]*dTBdx[k])/TB2);
    dFdxi[j*nvert+k]=cimag(dp*scale*(FTdx[j*nvert+k]*TB-F[j]*dTBdx[k])/TB2);
    dFdyr[j*nvert+k]=creal(dp*scale*(FTdy[j*nvert+k]*TB-F[j]*dTBdy[k])/TB2);
    dFdyi[j*nvert+k]=cimag(dp*scale*(FTdy[j*nvert+k]*TB-F[j]*dTBdy[k])/TB2);
    dFdzr[j*nvert+k]=creal(dp*scale*(FTdz[j*nvert+k]*TB-F[j]*dTBdz[k])/TB2);
    dFdzi[j*nvert+k]=cimag(dp*scale*(FTdz[j*nvert+k]*TB-F[j]*dTBdz[k])/TB2);
  }
  dFdAr[j*3+0]=creal(scale*(FTdA[j*3+0]*TB-F[j]*dTBdA[0])/TB2);
  dFdAi[j*3+0]=cimag(scale*(FTdA[j*3+0]*TB-F[j]*dTBdA[0])/TB2);
  dFdAr[j*3+1]=creal(scale*(FTdA[j*3+1]*TB-F[j]*dTBdA[1])/TB2);
  dFdAi[j*3+1]=cimag(scale*(FTdA[j*3+1]*TB-F[j]*dTBdA[1])/TB2);
  dFdAr[j*3+2]=creal(scale*(FTdA[j*3+2]*TB-F[j]*dTBdA[2])/TB2);
  dFdAi[j*3+2]=cimag(scale*(FTdA[j*3+2]*TB-F[j]*dTBdA[2])/TB2);
  temp=scale*F[j]/TB;
  Fr[j]=creal(temp);
  Fi[j]=cimag(temp);
  dFdoffr[j*2+0]=creal(2.0*I*PI*freqx[j]*temp);
  dFdoffi[j*2+0]=cimag(2.0*I*PI*freqx[j]*temp);
  dFdoffr[j*2+1]=creal(2.0*I*PI*freqy[j]*temp);
  dFdoffi[j*2+1]=cimag(2.0*I*PI*freqy[j]*temp);
}
if(albedo==1 && INI_FIT_AO_ALBEDO==1)
{
    for(int k=0;k<nvert;k++)
        for(int j=0;j<nfreq;j++)
        {
            scale=cexp(2.0*PI*I*(offset[0]*freqx[j]+offset[1]*freqy[j]));
            
            temp=scale*F[j]/TB;
            Fr[j]=creal(temp);
            Fi[j]=cimag(temp);
            temp=scale*(dAlb[j*nvert+k]*TB-F[j]*dAlbTB[k])/TB2;
            dAr[j*nvert+k]=creal(temp);
            dAi[j*nvert+k]=cimag(temp);
        }
        free(dAlb);
        free(dAlbTB);
}
free(FTdx);
free(FTdy);
free(FTdz);
free(FTdA);
free(dTBdx);
free(dTBdy);
free(dTBdz);
free(F);
free(FTda);
free(FTdb);
free(FTdc);
free(FTdd);
free(FTdg);
free(FTdh);
free(F0);
free(visible);
//diff=clock()-start;
//int msec=diff*1000/CLOCKS_PER_SEC;
//printf("TIME taken: %d seconds\n",msec/1000);
}


