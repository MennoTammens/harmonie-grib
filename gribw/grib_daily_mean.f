        program dailym
c
c	a program calc daily means grib -> grib
c
c
c	file 50: seq grib file-x
c	file 51: index grib file-x
c	file 54: list of times [yymmdd00]
c	file 55: var <cr> level
c
c	file 1: output file seq grib
c
c	v 1.1 7/29/94 minor mod to grib desc .. added nsum to ave of anal/fcst
c		change to setbit(....) .. assumes mask in constant in time
c	v 1.2 3/95 assumes mask can change in time needed for cloud temp
c
        parameter (maxv=100000)

        logical lbms(maxv), hasbit
        real data(maxv), mean(maxv)
        integer icount, count(maxv)

        integer kpds(100),kgds(100),recl,offset
        integer kpds6, kpds7

        integer id, time, time2, year, mon, hour, day
        real level
        character name*8

        read(55,'(a8)') name
        read(55,*) level
        close(55)

c	find kpds(5..7) parameters

        call atok5(name,id)
        if (id.eq.-1) then
            write(*,*) 'illegal variable name ', name
            call exit(8)
        endif

        call setlev(kpds,kgds,level)
        kpds6=kpds(6)
        kpds7=kpds(7)

	write(*,*) 'dailym ',name,'->',id,' level=',level

        inum = 0
        icount = 0

250	read(54,'(i6)',end=500) time
            if (time.le.0) goto 500

            time2 = time
            day = mod(time,100)
            time = time/100
            mon = mod(time,100)
            time = time/100
            year = mod(time,100)

            do 255 i = 1, maxv
		count(i) = 0
                mean(i) = 0.0
255         continue
            nsum = 0

            do 480 hour = 0, 18, 6

                do 260 i = 1, 100
                    kpds(i) = -1
260             continue
                kpds(5) = id
                kpds(6) = kpds6
                kpds(7) = kpds7
                kpds(8) = year
                kpds(9) = mon
                kpds(10) = day
                kpds(11) = hour

                call pdscan(51,kpds,inum,offset,recl)
                if (recl.eq.0) then
                    write(*,*) 'missing time ', time2, hour
		    call exit(8)
                    goto 480
                endif

                call rdgb(50,recl,offset,kpds,kgds,ndata,lbms,data)

                call getbit(kpds, kgds, hasbit)

                nx = kgds(2)
                ny = kgds(3)
                nxny = nx * ny

                if (hasbit) then
                    do 270 i = 1, nxny
                        if (lbms(i)) then
			    mean(i) = mean(i) + data(i)
			    count(i) = count(i) + 1
			endif
270                 continue
                else
                    do 280 i = 1, nxny
                        mean(i) = mean(i) + data(i)
			count(i) = count(i) + 1
280                 continue
                endif
                nsum = nsum + 1
480         continue

            if (nsum.eq.4) then
c               *** find daily ave and write grib record ***
                do 490 i = 1, nxny
		    hasbit = .false.
		    if (count(i).gt.0) then
			mean(i) = mean(i) / float(count(i))
			lbms(i) = .true.
		    else
			lbms(i) = .false.
			hasbit = .true.
		    endif
490             continue
c               set hour to 00Z
                kpds(11) = 0
c		set unit to hours
                kpds(13) = 1

	        if (kpds(16).eq.3) then
c		    average values
		    kpds(14) = 0
		    kpds(15) = 24
		    kpds(17) = nsum
c                   kpds(20) = 4 - nsum
		else if (kpds(16).eq.10) then
c		    forecast or analysis
	            kpds(16) = 113
		    kpds(15) = 6
		    kpds(17) = nsum
c                   kpds(20) = 4 - nsum
		else
		    write(*,*) 'unexpected Time Range in pds'
		    write(*,*) 'need to extend program'
		    call exit(8)
		endif

		call setbit(kpds,kgds,hasbit)

                call ezgbwk(mean,lbms,kpds,kgds,1)
                icount = icount + 1
            else
                write(*,*) 'did not read - not enough data:',time2
            endif
        goto 250

500	continue

        write(*,*) 'no. of daily fields writen ',icount

        if (icount.eq.0) call exit(8)
        call exit(0)
        stop
        end
