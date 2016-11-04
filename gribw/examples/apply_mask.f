c
c	This program applies a monthly mask to a data field
c	Used by a process to mask out GRIB data. See apply_mask.txt.
c
c    Fortran program reads (a) oldbin (b) dates (c) mask
c    and writes (a) newbin
c

	parameter(nxny=192*94)
	parameter(nmask=12)
	parameter(undef=9.999e20)

	real mask(nxny,nmask), array(nxny)
	integer date

c		open files

	open(unit=50,file='dates',status='old')
	open(unit=51,file='mask',form='unformatted',status='old')
	open(unit=52,file='oldbin',form='unformatted',status='old')
	open(unit=1,file='newbin',form='unformatted',status='unknown')

c		read mask

	do i = 1, nmask
	    read(51) (mask(j,i),j=1,nxny)
	enddo
	close(51)
	write(*,*) 'finished reading mask'

100	read(50,'(2x,i8)',end=200) date
	    read(52,end=300) array
	    month = mod(date/10000,100)
	    write(*,*) 'month=',month
	    do i = 1, nxny
		if (mask(i,month).eq.0.0) then
		    array(i) = undef
		endif
	    enddo
	    write(1) array
	    goto 100
200	continue
	close(1)
	close(50)
	close(52)
	write(*,*) 'done'
	stop
300	write(*,*) 'error in files'
	stop
	end
