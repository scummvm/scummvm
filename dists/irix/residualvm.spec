product residualvm
    id "ResidualVM 0.4.0git"
    image sw
        id "software"
        version 18
        order 9999
        subsys eoe default
            id "execution only env"
            replaces self
            exp residualvm.sw.eoe
        endsubsys
    endimage
    image man
        id "man pages"
        version 18
        order 9999
        subsys readme default
            id "residualvm documentation"
            replaces self
            exp residualvm.man.readme
        endsubsys
    endimage
endproduct
