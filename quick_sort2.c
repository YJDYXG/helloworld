#include <stdio.h>
#define Maxsize 50000
//保存每次分解之后的低位和高位
typedef struct node	
{      		
    int low;
    int high;	
}node;
node st[Maxsize];
void quicksort(int a[],int n)
{     	
	int i,j,low,high,temp,top=-1;
	//存放低位和高位数组的下标
	top++;	
	st[top].low=0;
	st[top].high=n-1;	
	while(top>-1)	
	{    		
	  low=st[top].low;
	  high=st[top].high;		
	  top--;		
	  i=low;j=high;		
	  if(low<high)		
	  {      			
		  temp=a[low];			
		  while(i!=j)			
		  {    				
			  while(i<j&&a[j]>temp)
			      j--;				
			  if(i<j)				
			  {					
				  a[i]=a[j];
                  i++;
			  }				
			  while(i<j&&a[i]<temp)
			      i++;
			  if(i<j)
			  {					
				  a[j]=a[i];
                  j--;
			  }			
		  }			
		  a[i]=temp;			
		  top++;
		  st[top].low=low;
		  st[top].high=i-1;			
		  top++;
		  st[top].low=i+1;
		  st[top].high=high;		
	 }	
  }
}

int main()
{	
	//int a[]={49,65,97,12,23,41,56,14};
	char *a[] = {"abc","Bca","AAA","abC","ABCD","BCa"};
	quicksort(a,6);
	int i=0;
	for(i=0;i<6;i++)
	{
		printf("%s\t",a[i]);
	}
	printf("\n");
    return 0;
}
