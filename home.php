<!-- insert fancy DOCTYPE here -->
<html>
<head>
	<title>GUSANOS - a liero clone</title>
	<link rel="stylesheet" href="style/style.css"/>
	<!--[if lt IE 7]>
		<link rel="stylesheet" href="style/ie.css"/>
	<![endif]-->
	<script>
		onload=function()
		{
			aa = document.getElementById('menu').getElementsByTagName('a')
			for (var i=0; i<aa.length; i++) if (aa[i].href==location) {
				aa[i].className = 'current'
				break
			}
		}
	</script>
</head>
<body>
	<div id="logo">
		<a href="home.php"><img src="img/logohover.png"/></a>
	</div>
	
	<div id="menu">
		<a href="home.php">home</a>
		<a href="news.php">news</a>
		<a href="http://sourceforge.net/projects/gusanos">get</a>
		<a href="documentation/doku/doku.php?id=main:main">docs</a>
		<a href="http://comser.liero.org.pl/forum">forum</a>
		<a href="http://www.cafepress.com/gusanos">shop</a>
	</div>
	
	<div id="links">
		<a href="http://www.boost.org"><img src="img/boost.png"/></a>
		<a href="http://www.talula.demon.co.uk/allegro"><img src="img/allegro.png"/></a>
		<a href="http://www.fmod.org"><img src="img/fmod.png"/></a>
		<a href="http://www.zoidcom.com"><img src="img/zoidcom.png"/></a>
		<a href="http://www.lua.org"><img src="img/lua.png"/></a>
		<a href="http://www.sf.net"><img src="img/sf.png"/></a>
		<a href="home.php#" class="liero"><img src="img/liero.png"/></a>	</div>
	<div id="sidebar">
	</div>
	
	<div id="content">
		<div id="introduction">
			<img style="float: right; margin-right: -20px; width: 265px; height: 372px" width="265" height="372" src="img/screenshots.jpg">
			<p id="intro1">A <strong>free</strong>, <strong>network-enabled</strong>, <strong>moddable</strong> Liero clone.</p>
			<p id="intro2">As <strong>Real-time</strong> Worms or <strong>2D</strong> Quake.</p>
			<p id="intro3">Play it on a <strong>LAN</strong>, <strong>online</strong>, <strong>one-on-one</strong> and against <strong>bots</strong>.</p>
		</div>
		
		<div id="download">
			<a id="get" href="http://prdownloads.sourceforge.net/gusanos/gusanos-0.9c.zip?download">download 0.9c!</a>
			<span id="platform">For Windows (5.2 mb) | <a href="http://sourceforge.net/projects/gusanos">other platforms</a></span>
		</div>
		
		<h2>Tell me more</h2>
		
		<p><span class="gusanos">GUSANOS</span> is an open-source clone of the DOS game <em>Liero</em>. It currently works on Windows and Linux.</p>
		
		<p>In the game each player controls a small worm armed to the teeth and fights other worms in a destructible battleground. It can be described as <em>"Real-time Worms"</em> or <em>"2D Quake"</em>, or a combination of both.</p>
		
		<p>The gameplay is as close as possible to Liero's. Hardcore Liero players will feel at home. Newbies, on the other hand, may find the controls hard to dominate at first, but not less enjoyable. Practice makes perfect.</p>
		
		<p><span class="gusanos">GUSANOS</span> improves greatly on what the original Liero left, adding modern graphics, audio and networking capabilites along with a myriad of other features.</p>
		
		<h2>Mod me beautiful</h2>
		
		<p>The <span class="gusanos">GUSANOS</span> engine is fully moddable. It achieves this in a Quake-like fashion, so that each mod lives in its own folder without messing with the core game files.</p>
		
		<p>Moddability is present at every level, from graphics to game logic. Hardcore modders get full power with the <a href="http://www.lua.org">LUA scripting language</a>. You can use the engine to create an entirely new game (<em>Total Conversion</em>), or simply add a few imaginative weapons.</p>
		
		<p>Go grab the documentation and get modding!</p>
		
		<h2>Free as in freak</h2>
		
		<p><span class="gusanos">GUSANOS</span> is, of course, free of charge*, free to distribute and free to modify.<p>
		
		<p>It is released under the <a href="http://www.gnu.org/copyleft/gpl.html">GNU General Public Licence</a> (GPL).</p>
		
		<p><em>* as long as we get free bandwidth</em></p>
		
			</div>

	<div id="footer">
		this <u>invalid</u> soup of (X)HTML by pilaf
		<!--[if IE]>.�o IE blows!<![endif]-->
	</div>
</body>
</html>